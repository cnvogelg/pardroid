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
#include "i2c.h"

#include "display.h"

int main(void)
{
  system_init();
  led_init();
  i2c_init();

  uart_init();
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
    system_wdt_reset();
    uart_send('.');
    led_on();
    timer_delay(200);
    system_wdt_reset();
    led_off();
    timer_delay(200);
  }

  // reset
  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  system_sys_reset();
  return 0;
}
