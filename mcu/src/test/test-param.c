#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "uart.h"
#include "uartutil.h"
#include "rominfo.h"
#include "system.h"
#include "led.h"
#include "timer.h"

#include "param.h"
#include "param_def.h"

int main(void)
{
  system_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-param!"));
  uart_send_crlf();

  rom_info();

  param_init();
  param_dump();

  // check should be ok
  uart_send_pstring(PSTR("check: "));
  u08 ok = param_check();
  uart_send_hex_byte(ok);
  uart_send_crlf();

  // set some values
  u08 mac[6] = { 1,2,3,4,5 };
  param_set_net_mac_addr(mac);
  u08 ip[4] = { 0x11,0x12,0x13,0x14 };
  param_set_net_ip_addr(ip);
  u08 mask[4] = { 0xee,0xdd,0xcc,0xbb };
  param_set_net_ip_mask(mask);
  param_set_net_dev(0x23);
  param_set_blk_dev(0x42);
  param_set_net_mtu(0x100);

  param_dump();

  // check should now be invalid
  uart_send_pstring(PSTR("check: "));
  ok = param_check();
  uart_send_hex_byte(ok);
  uart_send_crlf();

  uart_send_pstring(PSTR("sync"));
  uart_send_crlf();
  param_sync();
  param_dump();

  // check should be ok
  uart_send_pstring(PSTR("check: "));
  ok = param_check();
  uart_send_hex_byte(ok);
  uart_send_crlf();

  // reset param
  uart_send_pstring(PSTR("reset"));
  uart_send_crlf();
  param_reset();
  param_dump();

  // done. wait + reset
  for(u08 i=0;i<100;i++) {
    system_wdt_reset();
    uart_send('.');
    timer_delay(100);
  }

  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  system_sys_reset();
  return 0;
}
