#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "debug.h"

#include "uart.h"
#include "uartutil.h"
#include "rominfo.h"
#include "system.h"
#include "led.h"
#include "timer.h"

#include "proto_boot.h"
#include "proto_boot_shared.h"
#include "proto_boot_test_shared.h"

#include "fwid.h"
#include "fw_info.h"

FW_INFO(FWID_TEST_PROTO_BOOT, VERSION_TAG)

static u08 data_buf[TEST_PAGE_SIZE];

// API

u16 proto_boot_api_get_page_size(void)
{
  uart_send_pstring(PSTR("page size="));
  uart_send_hex_word(TEST_PAGE_SIZE);
  uart_send_crlf();

  return TEST_PAGE_SIZE;
}

u32 proto_boot_api_get_rom_size(void)
{
  uart_send_pstring(PSTR("rom size="));
  uart_send_hex_long(TEST_ROM_SIZE);
  uart_send_crlf();

  return TEST_ROM_SIZE;
}

u16 proto_boot_api_get_rom_crc(void)
{
  uart_send_pstring(PSTR("rom crc="));
  uart_send_hex_word(TEST_ROM_CRC);
  uart_send_crlf();

  return TEST_ROM_CRC;
}

u16 proto_boot_api_get_rom_mach_tag(void)
{
  uart_send_pstring(PSTR("rom mach tag="));
  uart_send_hex_word(TEST_ROM_MACH_TAG);
  uart_send_crlf();

  return TEST_ROM_MACH_TAG;
}

u16 proto_boot_api_get_rom_fw_version(void)
{
  uart_send_pstring(PSTR("rom fw version="));
  uart_send_hex_word(TEST_ROM_FW_VERSION);
  uart_send_crlf();

  return TEST_ROM_FW_VERSION;
}

u16 proto_boot_api_get_rom_fw_id(void)
{
  uart_send_pstring(PSTR("rom fw id="));
  uart_send_hex_word(TEST_ROM_FW_ID);
  uart_send_crlf();

  return TEST_ROM_FW_ID;
}

void proto_boot_api_set_page_addr(u32 addr)
{
  uart_send_pstring(PSTR("page addr="));
  uart_send_hex_long(TEST_ROM_FW_ID);
  uart_send_crlf();

  // write in buffer for retrieval by test
  data_buf[0] = (u08)((addr >> 24) & 0xff);
  data_buf[1] = (u08)((addr >> 16) & 0xff);
  data_buf[2] = (u08)((addr >> 8) & 0xff);
  data_buf[3] = (u08)(addr & 0xff);
}

void proto_boot_api_get_page_read_buf(u08 **buf, u16 *size)
{
  uart_send_pstring(PSTR("read page="));
  uart_send_crlf();

  *buf = data_buf;
  *size = TEST_PAGE_SIZE;
}

void proto_boot_api_get_page_write_buf(u08 **buf, u16 *size)
{
  uart_send_pstring(PSTR("write page"));
  uart_send_crlf();

  *buf = data_buf;
  *size = TEST_PAGE_SIZE;
}

void proto_boot_api_flash_page(void)
{
  uart_send_pstring(PSTR("flash page"));
  uart_send_crlf();
}

// ----- main -----

int main(void)
{
  system_init();
  led_init();

  uart_init();
  uart_send_pstring(PSTR("\r\n\r\n-----\r\nparbox: test-proto-boot!"));
  uart_send_crlf();

  rom_info();

  // wait for boot loader
  while(1) {
    if(proto_boot_init() == PROTO_BOOT_INIT_PROTO) {
      break;
    }

    uart_send('.');
    timer_delay(200);

    system_wdt_reset();
  }

  // failed to enter bootloader...
  uart_send_pstring(PSTR("enter boot"));
  uart_send_crlf();


  while(1) {
    // handle all proto io commands
    proto_boot_handle_cmd();

    // keep watchdog happy
    system_wdt_reset();
  }

  return 0;
}
