#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "bootbase.h"
#include "pablo.h"
#include "hw_uart.h"
#include "proto_boot.h"
#include "flash.h"
#include "machtag.h"
#include "fwid.h"
#include "fw_info.h"
#include "pablo.h"

#include "uartutil.h"

static u16 page_bytes;
static u32 page_addr;
static u08 *page_buf;

FW_INFO(FWID_BOOTLOADER_PABLO, VERSION_TAG)

// ----- main -----

void bootbase_main(u16 page_size, u08 *buf_ptr)
{
  page_bytes = page_size;
  page_buf = buf_ptr;

  // say hello
  hw_uart_init();
  hw_uart_send('P');

  // setup proto
  int res = proto_boot_init();
  hw_uart_send('A');

  // we need to launch the app code
  if(res == PROTO_BOOT_INIT_APP) {
    // check crc
    hw_uart_send('B');
    u16 crc = pablo_check_rom_crc();
    if(crc == 0) {
      hw_uart_send('L');
      // ensure that mach_tag matches in pablo footer
      u16 rom_mach_tag = pablo_get_mach_tag();
      if(rom_mach_tag == MACHTAG) {
        hw_uart_send('O');
        // run app if valid -> run it
        return;
      }
    }
    // something failed with app? -> wait for bootloader command
    proto_boot_wait();
  }

  // enter main loop of bootloader
  uart_send(':');
  while(1) {
    proto_boot_handle_cmd();
    boot_wdt_reset();
  }
}

// ----- boot API -----

u16 proto_boot_api_get_page_size(void)
{
  hw_uart_send('p');
  return page_bytes;
}

u32 proto_boot_api_get_rom_size(void)
{
  hw_uart_send('r');
  return CONFIG_MAX_ROM;
}

#define BIND(x) __attribute__ ((weak, alias(#x)))

u16 proto_boot_api_get_rom_crc(void)
{
  return pablo_get_rom_crc();
}

u16 proto_boot_api_get_rom_mach_tag(void)
{
  return pablo_get_mach_tag();
}

u16 proto_boot_api_get_rom_fw_version(void)
{
  return pablo_get_rom_version();
}

u16 proto_boot_api_get_rom_fw_id(void)
{
  return pablo_get_rom_fw_id();
}

void proto_boot_api_set_page_addr(u32 addr)
{
  hw_uart_send('a');
  page_addr = addr;
}

void proto_boot_api_get_page_read_buf(u08 **buf, u16 *size)
{
  *buf = page_buf;
  *size = page_bytes;
  
  hw_uart_send('r');
  flash_read_page(page_addr, page_buf);
}

void proto_boot_api_get_page_write_buf(u08 **buf, u16 *size)
{
  *buf = page_buf;
  *size = page_bytes;

  hw_uart_send('w');
}

void proto_boot_api_flash_page(void)
{
  hw_uart_send('(');
  flash_program_page(page_addr, page_buf);
  hw_uart_send(')'); 
}
