#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "hw_uart.h"
#include "uartutil.h"
#include "rominfo.h"
#include "hw_system.h"

#include "hw_timer.h"
#include "hw_spi.h"
#include "sdcard.h"

#include "fwid.h"
#include "fw_info.h"

FW_INFO(FWID_TEST_SDCARD, VERSION_TAG)

static u08 sdbuf[512];

static void test_sdcard(void)
{
  hw_spi_init();

  // init card
  uart_send_pstring(PSTR("sdcard: "));
  u32 start = hw_timer_millis();
  u08 res = sdcard_init();
  u32 end = hw_timer_millis();
  uart_send_hex_long(end - start);
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
      u32 start = hw_timer_millis();
      res = sdcard_read(i, sdbuf);
      u32 end = hw_timer_millis();
      if(res == SDCARD_RESULT_OK) {
        uart_send_hex_long(end - start);
        uart_send(',');
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
    u32 start = hw_timer_millis();
    res = sdcard_write(block, sdbuf);
    u32 end = hw_timer_millis();
    if(res == SDCARD_RESULT_OK) {
      uart_send_hex_long(end - start);
      uart_send(',');
      // read back
      start = hw_timer_millis();
      res = sdcard_read(block, sdbuf);
      end = hw_timer_millis();
      if(res == SDCARD_RESULT_OK) {
        uart_send_hex_long(end - start);
        uart_send(',');
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

int main(void)
{
  hw_system_init();

  hw_uart_init();
  uart_send_pstring(PSTR("parbox: test-sdcard!"));
  uart_send_crlf();

  rom_info();

  test_sdcard();

  for(int i=0;i<100;i++) {
    uart_send('.');
    hw_timer_delay_ms(200);
  }

  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  hw_system_reset();
  return 0;
}
