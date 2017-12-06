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

#include "spi.h"
#include "wiznet.h"


static void test_wiznet(void)
{
  spi_init();

  uart_send_pstring(PSTR("wiznet: "));
  wiznet_init();

  u08 mac[6] = { 0x1a,0x11,0xaf,0xa0,0x47,0x11 };
  wiznet_set_mac(mac);
  u08 addr[4] = { 192,168,2,42 };
  wiznet_set_src_addr(addr);
  u08 mask[4] = { 255,255,255,0 };
  wiznet_set_net_mask(mask);
  u08 gw[4] = { 192,168,2,1 };
  wiznet_set_src_addr(gw);

  uart_send_crlf();
}

int main(void)
{
  system_init();
  //led_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-base!"));
  uart_send_crlf();

  rom_info();

  test_wiznet();

  for(int i=0;i<100;i++) {
    system_wdt_reset();
    u08 up = wiznet_is_link_up();
    if(up) {
      uart_send('*');
    } else {
      uart_send('.');
    }
    timer_delay(200);
  }

  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  system_sys_reset();
  return 0;
}
