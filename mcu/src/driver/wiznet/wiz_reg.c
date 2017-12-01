#include "autoconf.h"
#include "types.h"

#include "spi.h"

#include "wiz_reg.h"

#if CONFIG_DRIVER_WIZNET_SPI_CS == 0
#define spi_enable_cs()   spi_enable_cs0()
#define spi_disable_cs()  spi_disable_cs0()
#elif CONFIG_DRIVER_WIZNET_SPI_CS == 1
#define spi_enable_cs()   spi_enable_cs1()
#define spi_disable_cs()  spi_disable_cs1()
#else
#error invalid CONFIG_DRIVER_WIZNET_SPI_CS
#endif

#define SOCK_BASE    0x4000
#define SOCK_SIZE    0x0100


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

void wiz_reg_base_write(u16 addr, u08 value)
{
  return write(addr, value);
}

void wiz_reg_base_write_buf(u16 addr, const u08 *buf, u16 len)
{
  return write_buf(addr, buf, len);
}

u08  wiz_reg_base_read(u16 addr)
{
  return read(addr);
}

void wiz_reg_base_read_buf(u16 addr, u08 *buf, u16 len)
{
  read_buf(addr, buf, len);
}

void wiz_reg_socket_write(u08 sock, u16 addr, u08 value)
{
  u16 off = addr + SOCK_BASE + sock * SOCK_SIZE;
  write(off, value);
}

void wiz_reg_socket_write_buf(u08 sock, u16 addr, const u08 *buf, u16 len)
{
  u16 off = addr + SOCK_BASE + sock * SOCK_SIZE;
  write_buf(off, buf, len);
}

u08  wiz_reg_socket_read(u08 sock, u16 addr)
{
  u16 off = addr + SOCK_BASE + sock * SOCK_SIZE;
  return read(off);
}

void wiz_reg_socket_read_buf(u08 sock, u16 addr, u08 *buf, u16 len)
{
  u16 off = addr + SOCK_BASE + sock * SOCK_SIZE;
  read_buf(off, buf, len);
}
