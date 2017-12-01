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

#include "proto_low.h"

// define the tests you want
//#define TEST_WAIT_WATCHDOG
//#define TEST_SYS_RESET
//#define TEST_TIMER_TIMEOUT
//#define TEST_SDCARD
//#define TEST_SPI
//#define TEST_WIZNET

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

#ifdef TEST_SDCARD
#include "spi.h"
#include "sdcard.h"

static u08 sdbuf[512];

static void test_sdcard(void)
{
  spi_init();

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
    const u32 block = 100;
    for(u16 i=0;i<512;i++) {
      sdbuf[i] = (u08)(i & 0xff);
    }
    uart_send_pstring(PSTR("write: "));
    res = sdcard_write(block, sdbuf);
    if(res == SDCARD_RESULT_OK) {
      uart_send('.');
      // read back
      res = sdcard_read(block, sdbuf);
      if(res == SDCARD_RESULT_OK) {
        uart_send('.');
        for(u16 i=0;i<512;i++) {
          u08 d = (u08)(i & 0xff);
          if(d != sdbuf[i]) {
            uart_send('#');
          }
        }
      } else {
        uart_send('?');
      }
    } else {
      uart_send_hex_byte(res);
      uart_send('!');
    }
    uart_send_crlf();
  }
}
#endif

#ifdef TEST_SPI
#include "spi.h"

static void test_spi(void)
{
  spi_init();
  spi_set_speed(SPI_SPEED_MAX);

  spi_enable_cs0();
  spi_out(0xaa);
  spi_out(0xaa);
  spi_out(0xaa);
  spi_disable_cs0();

  timer_delay(10);

  spi_enable_cs0();
  spi_out(0x55);
  spi_disable_cs0();

  timer_delay(20);

  spi_enable_cs1();
  spi_out(0xaa);
  spi_disable_cs1();

  timer_delay(10);

  spi_enable_cs1();
  spi_out(0x55);
  spi_disable_cs1();

  timer_delay(20);

  spi_set_speed(SPI_SPEED_SLOW);

  spi_enable_cs0();
  spi_out(0xaa);
  spi_out(0xaa);
  spi_out(0xaa);
  spi_disable_cs0();

  timer_delay(10);

  spi_enable_cs0();
  spi_out(0x55);
  spi_disable_cs0();

  timer_delay(20);

  spi_enable_cs1();
  spi_out(0xaa);
  spi_disable_cs1();

  timer_delay(10);

  spi_enable_cs1();
  spi_out(0x55);
  spi_disable_cs1();
}
#endif

#ifdef TEST_WIZNET
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
#endif

int main(void)
{
  system_init();
  //led_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-base!"));
  uart_send_crlf();

  rom_info();

  //proto_low_init(0);

#ifdef TEST_WAIT_WATCHDOG
  test_wait_watchdog();
#endif
#ifdef TEST_SYS_RESET
  test_sys_reset();
#endif
#ifdef TEST_TIMER_TIMEOUT
  test_timer_timeout();
#endif

#ifdef TEST_SPI
  test_spi();
#endif
#ifdef TEST_SDCARD
  test_sdcard();
#endif
#ifdef TEST_WIZNET
  test_wiznet();
#endif

  for(int i=0;i<100;i++) {
    system_wdt_reset();
#ifdef TEST_WIZNET
    u08 up = wiznet_is_link_up();
    if(up) {
      uart_send('*');
    } else {
      uart_send('.');
    }
#else
    uart_send('.');
#endif
    timer_delay(200);
  }

  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  system_sys_reset();
  return 0;
}
