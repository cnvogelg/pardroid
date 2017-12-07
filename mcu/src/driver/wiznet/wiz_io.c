#include "autoconf.h"
#include "types.h"

#define DEBUG 0
#include "debug.h"

#include "spi.h"

#include "wiz_io.h"
#include "wiz_defs.h"

// configure socket count
#define NUM_SOCKETS  CONFIG_DRIVER_WIZNET_NUM_SOCKETS
#define MEM_SIZE     ((32 / NUM_SOCKETS) * 1024)
#define MEM_MASK     (MEM_SIZE - 1)
#define TX_BASE(s)   (0x8000 + s * MEM_SIZE)
#define RX_BASE(s)   (0xc000 + s * MEM_SIZE)
#define SOCK_BASE    0x4000
#define SOCK_SIZE    0x0100


#if CONFIG_DRIVER_WIZNET_SPI_CS == 0
#define spi_enable_cs()   spi_enable_cs0()
#define spi_disable_cs()  spi_disable_cs0()
#elif CONFIG_DRIVER_WIZNET_SPI_CS == 1
#define spi_enable_cs()   spi_enable_cs1()
#define spi_disable_cs()  spi_disable_cs1()
#else
#error invalid CONFIG_DRIVER_WIZNET_SPI_CS
#endif


static void write(u16 addr, u08 data)
{
  spi_enable_cs();
  spi_out(addr >> 8);
  spi_out(addr & 0xff);
  spi_out(0x80);
  spi_out(0x01);
  spi_out(data);
  spi_disable_cs();
}

static u08 read(u16 addr)
{
  spi_enable_cs();
  spi_out(addr >> 8);
  spi_out(addr & 0xff);
  spi_out(0x00);
  spi_out(0x01);
  u08 data = spi_in();
  spi_disable_cs();
  return data;
}

static void write_buf(u16 addr, const u08 *buf, u16 len)
{
  spi_enable_cs();
  spi_out(addr >> 8);
  spi_out(addr & 0xff);
  spi_out(0x80 | ((len & 0x7f00) >> 8));
  spi_out(len & 0xff);
  for(u16 i=0;i<len;i++) {
    spi_out(*buf++);
  }
  spi_disable_cs();
}

static void read_buf(u16 addr, u08 *buf, u16 len)
{
  spi_enable_cs();
  spi_out(addr >> 8);
  spi_out(addr & 0xff);
  spi_out((len & 0x7f00) >> 8);
  spi_out(len & 0xff);
  for(u16 i=0;i<len;i++) {
    *buf++ = spi_in();
  }
  spi_disable_cs();
}

void wiz_io_base_reg_write(u16 addr, u08 value)
{
  return write(addr, value);
}

void wiz_io_base_reg_write_buf(u16 addr, const u08 *buf, u16 len)
{
  return write_buf(addr, buf, len);
}

u08  wiz_io_base_reg_read(u16 addr)
{
  return read(addr);
}

void wiz_io_base_reg_read_buf(u16 addr, u08 *buf, u16 len)
{
  read_buf(addr, buf, len);
}

void wiz_io_socket_reg_write(u08 sock, u16 addr, u08 value)
{
  u16 off = addr + SOCK_BASE + sock * SOCK_SIZE;
  write(off, value);
}

void wiz_io_socket_reg_write_word(u08 sock, u16 addr, u16 value)
{
  u08 buf[2];
  buf[0] = (u08)(value >> 8);
  buf[1] = (u08)(value & 0xff);
  wiz_io_socket_reg_write_buf(sock, addr, buf, 2);
}

void wiz_io_socket_reg_write_buf(u08 sock, u16 addr, const u08 *buf, u16 len)
{
  u16 off = addr + SOCK_BASE + sock * SOCK_SIZE;
  write_buf(off, buf, len);
}

u08  wiz_io_socket_reg_read(u08 sock, u16 addr)
{
  u16 off = addr + SOCK_BASE + sock * SOCK_SIZE;
  return read(off);
}

u16  wiz_io_socket_reg_read_word(u08 sock, u16 addr)
{
  u08 buf[2];
  wiz_io_socket_reg_read_buf(sock, addr, buf, 2);
  return buf[0] << 8 | buf[1];
}

void wiz_io_socket_reg_read_buf(u08 sock, u16 addr, u08 *buf, u16 len)
{
  u16 off = addr + SOCK_BASE + sock * SOCK_SIZE;
  read_buf(off, buf, len);
}

/* buffer ops */

u16 wiz_io_get_rx_size(u08 sock)
{
  u16 v0=0, v1=0;
  do {
    v1 = wiz_io_socket_reg_read_word(sock, WIZ_REG_SOCKET_RX_SIZE);
    if(v1 != 0) {
      v0 = wiz_io_socket_reg_read_word(sock, WIZ_REG_SOCKET_RX_SIZE);
    }
  }
  while(v0 != v1);
  return v0;
}

u16 wiz_io_get_tx_free(u08 sock)
{
  u16 v0=0, v1=0;
  do {
    v1 = wiz_io_socket_reg_read_word(sock, WIZ_REG_SOCKET_TX_FREE);
    if(v1 != 0) {
      v0 = wiz_io_socket_reg_read_word(sock, WIZ_REG_SOCKET_TX_FREE);
    }
  }
  while(v0 != v1);
  return v0;
}

void wiz_io_tx_buffer_write(u08 sock, u16 offset, const u08 *buf, u16 len)
{
  u16 ptr = wiz_io_socket_reg_read_word(sock, WIZ_REG_SOCKET_TX_WRITE_PTR);
  ptr += offset;

  u16 rel_pos = ptr & MEM_MASK;
  u16 base = TX_BASE(sock);
  u16 addr = base + rel_pos;

  // wrap around
  if((rel_pos + len) > MEM_SIZE) {
    u16 size = MEM_SIZE - rel_pos;
    write_buf(addr, buf, size);
    write_buf(base, buf + size, len - size);
  } else {
    write_buf(addr, buf, len);
  }
}

void wiz_io_tx_buffer_confirm(u08 sock, u16 len)
{
  u16 ptr = wiz_io_socket_reg_read_word(sock, WIZ_REG_SOCKET_TX_WRITE_PTR);
  ptr += len;
  wiz_io_socket_reg_write_word(sock, WIZ_REG_SOCKET_TX_WRITE_PTR, ptr);
}

void wiz_io_rx_buffer_read(u08 sock, u16 offset, u08 *buf, u16 len)
{
  u16 ptr = wiz_io_socket_reg_read_word(sock, WIZ_REG_SOCKET_RX_READ_PTR);
  ptr += offset;

  u16 rel_pos = ptr & MEM_MASK;
  u16 base = RX_BASE(sock);
  u16 addr = base + rel_pos;

  // wrap around
  if((rel_pos + len) > MEM_SIZE) {
    u16 size = MEM_SIZE - rel_pos;
    read_buf(addr, buf, size);
    read_buf(base, buf + size, len - size);
  } else {
    read_buf(addr, buf, len);
  }
}

void wiz_io_rx_buffer_confirm(u08 sock, u16 len)
{
  u16 ptr = wiz_io_socket_reg_read_word(sock, WIZ_REG_SOCKET_RX_READ_PTR);
  ptr += len;
  wiz_io_socket_reg_write_word(sock, WIZ_REG_SOCKET_RX_READ_PTR, ptr);
}
