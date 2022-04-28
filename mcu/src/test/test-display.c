#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"
#include "fwid.h"
#include "fw_info.h"

#include "hw_uart.h"
#include "hw_system.h"
#include "hw_led.h"
#include "hw_timer.h"
#include "hw_i2c.h"

#include "uartutil.h"
#include "rominfo.h"

#include "display.h"

FW_INFO(FWID_TEST_DISPLAY, VERSION_TAG)

int main(void)
{
  hw_system_init();
  hw_led_init();
  hw_i2c_init();
  hw_uart_init();

  uart_send_pstring(PSTR("parbox: test-display!"));
  uart_send_crlf();

  rom_info();

  // display init
  uart_send_pstring(PSTR("display_init:"));
  u08 res = display_init();
  uart_send_hex_byte(res);
  uart_send_crlf();

  uart_send_pstring(PSTR("display_clear:"));
  res = display_clear();
  uart_send_hex_byte(res);
  uart_send_crlf();

  display_setpos(4,1);
  uart_send_pstring(PSTR("display_printp:"));
  res = display_printp(PSTR("hello, world!"));
  uart_send_hex_byte(res);
  uart_send_crlf();

  display_setpos(2,2);
  uart_send_pstring(PSTR("display_print:"));
  res = display_print("HI!");
  uart_send_hex_byte(res);
  uart_send_crlf();


  // blink a bit
  for(int i=0;i<10;i++) {
    hw_system_wdt_reset();
    uart_send('.');
    hw_led_on();
    hw_timer_delay_ms(200);
    hw_system_wdt_reset();
    hw_led_off();
    hw_timer_delay_ms(200);
  }

  // reset
  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  hw_system_sys_reset();
  return 0;
}
