#include "autoconf.h"
#include "types.h"

#define DEBUG CONFIG_DEBUG_DRIVER_WIZNET

#include "debug.h"

#include "wiznet.h"
#include "wiznet_low.h"
#include "wiz_io.h"
#include "wiz_defs.h"

#include "timer.h"
#include "uartutil.h"
#include "system.h"

// configure socket count
#define NUM_SOCKETS  CONFIG_DRIVER_WIZNET_NUM_SOCKETS
#define MEM_SIZE     (32 / NUM_SOCKETS)

#define WIZ_CMD_TIMEOUT   100

// ----- helper functions -----

static inline void set_src_port(u08 sock, u16 port)
{
  wiz_io_socket_reg_write_word(sock, WIZ_REG_SOCKET_SRC_PORT, port);
}

static inline void set_dest_addr(u08 sock, const u08 addr[4])
{
  wiz_io_socket_reg_write_buf(sock, WIZ_REG_SOCKET_DST_IP, addr, 4);
}

static inline void set_dest_port(u08 sock, u16 port)
{
  wiz_io_socket_reg_write_word(sock, WIZ_REG_SOCKET_DST_PORT, port);
}

static inline void write_ir(u08 sock, u08 val)
{
  wiz_io_socket_reg_write(sock, WIZ_REG_SOCKET_IR, val);
}

static inline u08 read_ir(u08 sock)
{
  return wiz_io_socket_reg_read(sock, WIZ_REG_SOCKET_IR);
}

static inline void write_mode(u08 sock, u08 mode)
{
  wiz_io_socket_reg_write(sock, WIZ_REG_SOCKET_MODE, mode);
}

static inline u08 read_mode(u08 sock)
{
  return wiz_io_socket_reg_read(sock, WIZ_REG_SOCKET_MODE);
}

static inline u08 read_status(u08 sock)
{
  return wiz_io_socket_reg_read(sock, WIZ_REG_SOCKET_STATUS);
}

u08 exec_cmd(u08 sock, u08 cmd)
{
  wiz_io_socket_reg_write(sock, WIZ_REG_SOCKET_CMD, cmd);

  // CMD reg is cleared if command was accepted
  timer_ms_t t0 = timer_millis();
  while(wiz_io_socket_reg_read(sock, WIZ_REG_SOCKET_CMD)) {
    if(timer_millis_timed_out(t0, WIZ_CMD_TIMEOUT)) {
      return WIZNET_RESULT_CMD_TIMEOUT;
    }
    system_wdt_reset();
  }
  return WIZNET_RESULT_OK;
}

// ----- API -----

void wiznet_init(void)
{
  wiznet_low_reset();
  timer_delay(200); // 150ms in data sheet WIZ820io

  wiz_io_base_reg_write(WIZ_REG_BASE_MODE, WIZ_MASK_RESET);

  // configure socket rx/tx buffer size
  u08 socket = 0;
  while(socket < NUM_SOCKETS) {
    wiz_io_socket_reg_write(socket, WIZ_REG_SOCKET_RX_MEMSIZE, MEM_SIZE);
    wiz_io_socket_reg_write(socket, WIZ_REG_SOCKET_TX_MEMSIZE, MEM_SIZE);
    socket++;
  }
  while(socket < WIZ_MAX_SOCKETS) {
    wiz_io_socket_reg_write(socket, WIZ_REG_SOCKET_RX_MEMSIZE, 0);
    wiz_io_socket_reg_write(socket, WIZ_REG_SOCKET_TX_MEMSIZE, 0);
    socket++;
  }
}

void wiznet_set_mac(const u08 mac[6])
{
  wiz_io_base_reg_write_buf(WIZ_REG_BASE_SRC_MAC, mac, 6);
}

void wiznet_set_src_addr(const u08 addr[4])
{
  wiz_io_base_reg_write_buf(WIZ_REG_BASE_SRC_ADDR, addr, 4);
}

void wiznet_set_net_mask(const u08 mask[4])
{
  wiz_io_base_reg_write_buf(WIZ_REG_BASE_NET_MASK, mask, 4);
}

void wiznet_set_gw_addr(const u08 addr[4])
{
  wiz_io_base_reg_write_buf(WIZ_REG_BASE_GW_ADDR, addr, 4);
}

void wiznet_get_mac(u08 mac[6])
{
  wiz_io_base_reg_read_buf(WIZ_REG_BASE_SRC_MAC, mac, 6);
}

void wiznet_get_src_addr(u08 addr[4])
{
  wiz_io_base_reg_read_buf(WIZ_REG_BASE_SRC_ADDR, addr, 4);
}

void wiznet_get_net_mask(u08 mask[4])
{
  wiz_io_base_reg_read_buf(WIZ_REG_BASE_NET_MASK, mask, 4);
}

void wiznet_get_gw_addr(u08 addr[4])
{
  wiz_io_base_reg_read_buf(WIZ_REG_BASE_GW_ADDR, addr, 4);
}

u08 wiznet_is_link_up(void)
{
  u08 val = wiz_io_base_reg_read(WIZ_REG_BASE_PHY_STATUS);
  return (val & WIZ_MASK_LINK_UP) == WIZ_MASK_LINK_UP;
}

u08 wiznet_find_free_socket()
{
  for(u08 socket=0;socket < NUM_SOCKETS; socket++) {
    u08 mode = read_mode(socket);
    if((mode & WIZ_SOCKET_MODE_MASK) == WIZ_SOCKET_MODE_CLOSED) {
      return socket;
    }
  }
  return WIZNET_NO_SOCKET;
}

u08 wiznet_udp_open(u08 sock, u16 my_port)
{
  write_mode(sock, WIZ_SOCKET_MODE_UDP);
  set_src_port(sock, my_port);
  return exec_cmd(sock, WIZ_SOCKET_CMD_OPEN);
}

u08 wiznet_udp_close(u08 sock)
{
  u08 res = exec_cmd(sock, WIZ_SOCKET_CMD_CLOSE);
  write_ir(sock, 0xff);
  write_mode(sock, WIZ_SOCKET_MODE_CLOSED);
  return res;
}

u08 wiznet_udp_send(u08 sock, const u08 *buf, u16 len,
                    const u08 addr[4], u16 port)
{
  set_dest_addr(sock, addr);
  set_dest_port(sock, port);

  // copy data
  wiz_io_tx_buffer_write(sock, 0, buf, len, 1);

  u08 res = exec_cmd(sock, WIZ_SOCKET_CMD_SEND);
  if(res != WIZNET_RESULT_OK) {
    return res;
  }

  while(1) {
    u08 ir = read_ir(sock);
    if((ir & WIZ_SOCKET_IR_SEND_OK) == WIZ_SOCKET_IR_SEND_OK) {
      write_ir(sock, WIZ_SOCKET_IR_SEND_OK);
      return WIZNET_RESULT_OK;
    }
    if((ir & WIZ_SOCKET_IR_TIMEOUT) == WIZ_SOCKET_IR_TIMEOUT) {
      write_ir(sock, WIZ_SOCKET_IR_SEND_OK | WIZ_SOCKET_IR_TIMEOUT);
      return WIZNET_RESULT_SEND_TIMEOUT;
    }
  }
}
