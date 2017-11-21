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

#ifdef CONFIG_DRIVER_SDCARD
#include "spi.h"
#include "sdcard.h"
#endif

#include "proto_low.h"

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
    timer_delay(100);
  }
}
#endif

#ifdef TEST_SYS_RESET
static void test_sys_reset(void)
{
  uart_send_pstring(PSTR("reset!"));
  uart_send_crlf();
  system_sys_reset();
}
#endif

#ifdef TEST_TIMER_TIMEOUT
static void test_timer_timeout(void)
{
  timer_ms_t t1 = timer_millis();
  uart_send_pstring(PSTR("go "));
  uart_send_hex_word(t1);
  uart_send_crlf();
  while(!timer_millis_timed_out(t1, 10000)) {
    system_wdt_reset();
  }
  uart_send_pstring(PSTR("done"));
  uart_send_crlf();
}
#endif

#ifdef CONFIG_DRIVER_SDCARD
static u08 sdbuf[512];

static void test_sdcard(void)
{
  // init card
  uart_send_pstring(PSTR("sdcard: "));
  u08 res = sdcard_init();
  uart_send_pstring(PSTR(" -> "));
  uart_send_hex_byte(res);
  uart_send_crlf();
  if(res == SDCARD_RESULT_OK) {
    // get capacity
    uart_send_pstring(PSTR("capacity: "));
    u32 num_blocks;
    res = sdcard_get_capacity(&num_blocks);
    uart_send_pstring(PSTR(" -> "));
    if(res == SDCARD_RESULT_OK) {
      uart_send_hex_long(num_blocks);
    } else {
      uart_send_hex_byte(res);
    }
    uart_send_crlf();

    // read a block
    uart_send_pstring(PSTR("read: "));
    for(u08 i=0;i<10;i++) {
      res = sdcard_read(i, sdbuf);
      if(res == SDCARD_RESULT_OK) {
        uart_send('.');
      } else {
        uart_send_hex_byte(res);
        uart_send('!');
      }
    }
    uart_send_crlf();

    // write a block
    uart_send_pstring(PSTR("write: "));
    res = sdcard_read(1, sdbuf);
    if(res == SDCARD_RESULT_OK) {
      res = sdcard_write(1, sdbuf);
      if(res == SDCARD_RESULT_OK) {
        uart_send('.');
      } else {
        uart_send_hex_byte(res);
        uart_send('!');
      }
      uart_send_crlf();
    }
  }
}
#endif

int main(void)
{
  system_init();
  led_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-base!"));
  uart_send_crlf();

  rom_info();

  proto_low_init(0);

#ifdef TEST_WAIT_WATCHDOG
  test_wait_watchdog();
#endif
#ifdef TEST_SYS_RESET
  test_sys_reset();
#endif
#ifdef TEST_TIMER_TIMEOUT
  test_timer_timeout();
#endif

#ifdef CONFIG_SPI
  spi_init();
#endif
#ifdef CONFIG_DRIVER_SDCARD
  test_sdcard();
#endif

  for(int i=0;i<100;i++) {
    system_wdt_reset();
    uart_send('.');
    timer_delay(200);
  }

  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  system_sys_reset();
  return 0;
}
