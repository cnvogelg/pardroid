#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "hw_system.h"
#include "hw_led.h"
#include "hw_timer.h"

#include "uartutil.h"
#include "rominfo.h"
#include "fwid.h"
#include "fw_info.h"

#include <stdlib.h>

FW_INFO(FWID_TEST_BASE, VERSION_TAG)

// define the tests you want
//#define TEST_WAIT_WATCHDOG
//#define TEST_SYS_RESET
//#define TEST_TIMER_TIMEOUT

#ifdef TEST_WAIT_WATCHDOG
static void test_wait_watchdog(void)
{
  uart_send_pstring(PSTR("waiting for wd!"));
  uart_send_crlf();
  // test wd
  for(int i=0;i<100;i++) {
    uart_send_hex_byte(i);
    uart_send_crlf();
    hw_timer_delay_ms(100);
  }
}
#endif

#ifdef TEST_SYS_RESET
static void test_sys_reset(void)
{
  uart_send_pstring(PSTR("reset!"));
  uart_send_crlf();
  hw_system_reset();
}
#endif

#ifdef TEST_TIMER_TIMEOUT
static void test_timer_timeout(void)
{
  hw_timer_ms_t t1 = hw_timer_millis();
  uart_send_pstring(PSTR("go "));
  uart_send_hex_word(t1);
  uart_send_crlf();
  while(!hw_timer_millis_timed_out(t1, 10000)) {
  }
  uart_send_pstring(PSTR("done"));
  uart_send_crlf();
}
#endif

int main(void)
{
  hw_system_init();
  hw_led_init();

  hw_uart_init();
  uart_send_pstring(PSTR("parbox: test-base!"));
  uart_send_crlf();

  rom_info();

#ifdef TEST_WAIT_WATCHDOG
  test_wait_watchdog();
#endif
#ifdef TEST_SYS_RESET
  test_sys_reset();
#endif
#ifdef TEST_TIMER_TIMEOUT
  test_timer_timeout();
#endif

  /* malloc test */
  u08 *buf = (u08 *)malloc(512);
  uart_send_pstring(PSTR("malloc: "));
  uart_send_hex_ptr(buf);
  free(buf);
  uart_send_pstring(PSTR(" ... free"));
  uart_send_crlf();

  int on = 0;
  for(int i=0;i<100;i++) {
    uart_send('.');
    hw_timer_delay_ms(200);
    hw_led_set(on);
    on = !on;
  }

  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  hw_system_reset();
  return 0;
}
