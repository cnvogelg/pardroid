#include "autoconf.h"
#include "types.h"

#define DEBUG CONFIG_DEBUG_DRIVER_WIZNET

#include "debug.h"

#include "wiznet.h"
#include "wiznet_low.h"
#include "wiz_reg.h"
#include "wiz_defs.h"

#include "timer.h"
#include "uartutil.h"

void wiznet_init(void)
{
  wiznet_low_reset();
  timer_delay(200); // 150ms in data sheet WIZ820io

  wiz_reg_base_write(WIZ_REG_BASE_MODE, WIZ_MASK_RESET);
}

void wiznet_set_mac(const u08 mac[6])
{
  // write mac
  wiz_reg_base_write_buf(WIZ_REG_BASE_SRC_MAC, mac, 6);
}

void wiznet_set_src_addr(const u08 addr[4])
{
  wiz_reg_base_write_buf(WIZ_REG_BASE_SRC_ADDR, addr, 4);
}

void wiznet_set_net_mask(const u08 mask[4])
{
  wiz_reg_base_write_buf(WIZ_REG_BASE_NET_MASK, mask, 4);
}

void wiznet_set_gw_addr(const u08 addr[4])
{
  wiz_reg_base_write_buf(WIZ_REG_BASE_GW_ADDR, addr, 4);
}

u08 wiznet_is_link_up(void)
{
  u08 val = wiz_reg_base_read(WIZ_REG_BASE_PHY_STATUS);
  return (val & WIZ_MASK_LINK_UP) == WIZ_MASK_LINK_UP;
}

