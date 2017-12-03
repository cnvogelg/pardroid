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

// configure socket count
#define NUM_SOCKETS  CONFIG_DRIVER_WIZNET_NUM_SOCKETS
#define MEM_SIZE     (32 / NUM_SOCKETS)


void wiznet_init(void)
{
  wiznet_low_reset();
  timer_delay(200); // 150ms in data sheet WIZ820io

  wiz_io_base_reg_write(WIZ_REG_BASE_MODE, WIZ_MASK_RESET);

  // configure socket rx/tx buffer size
  u08 socket = 0;
  while(socket < NUM_SOCKETS) {
    wiz_io_socket_reg_write_word(socket, WIZ_REG_SOCKET_RX_MEMSIZE, MEM_SIZE);
    wiz_io_socket_reg_write_word(socket, WIZ_REG_SOCKET_TX_MEMSIZE, MEM_SIZE);
    socket++;
  }
  while(socket < WIZ_MAX_SOCKETS) {
    wiz_io_socket_reg_write_word(socket, WIZ_REG_SOCKET_RX_MEMSIZE, 0);
    wiz_io_socket_reg_write_word(socket, WIZ_REG_SOCKET_TX_MEMSIZE, 0);
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

u08 wiznet_is_link_up(void)
{
  u08 val = wiz_io_base_reg_read(WIZ_REG_BASE_PHY_STATUS);
  return (val & WIZ_MASK_LINK_UP) == WIZ_MASK_LINK_UP;
}

