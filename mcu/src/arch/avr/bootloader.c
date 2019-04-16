#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>

#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "bootloader.h"
#include "uart.h"
#include "proto.h"
#include "proto_low.h"
#include "flash.h"
#include "pablo.h"
#include "reg.h"
#include "machtag.h"

static u08 status;
static u16 page_addr;
static u08 page_buf[SPM_PAGESIZE];

// ----- actions -----

void proto_api_acion(u08 num)
{
  /* no extra action */
}

// ----- functions -----

u16  proto_api_wfunc_read(u08 num)
{
  switch(num) {
    case PROTO_WFUNC_BOOT_MAGIC:
      return PROTO_MAGIC_BOOTLOADER;
    case PROTO_WFUNC_BOOT_MACHTAG:
      return MACHTAG;
    case PROTO_WFUNC_BOOT_VERSION:
      return VERSION_TAG;
    case PROTO_WFUNC_BOOT_PAGE_SIZE:
      return SPM_PAGESIZE;
    case PROTO_WFUNC_BOOT_ROM_SIZE:
      return CONFIG_MAX_ROM;
    case PROTO_WFUNC_BOOT_ROM_CRC:
      return pablo_get_rom_crc();
    case PROTO_WFUNC_BOOT_ROM_MACHTAG:
      return pablo_get_mach_tag();
    case PROTO_WFUNC_BOOT_ROM_FW_VERSION:
      return pablo_get_rom_version();
    case PROTO_WFUNC_BOOT_ROM_FW_ID:
      return pablo_get_rom_fw_id();
    case PROTO_WFUNC_BOOT_PAGE_ADDR:
      return page_addr;
    default:
      return 0;
  }
}

void proto_api_wfunc_write(u08 num, u16 val)
{
  switch(num) {
    case PROTO_WFUNC_BOOT_PAGE_ADDR:
      page_addr = val;
      break;
  }
}

/* non used API */

u32 proto_api_lfunc_read(u08 num)
{
  return 0;
}

void proto_api_lfunc_write(u08 num, u32 val)
{
}

void proto_api_action(u08 num)
{
}

// from optiboot
static void run_app(u08 rstFlags) __attribute__ ((naked));
static void run_app(u08 rstFlags)
{
  // store reset reason
  __asm__ __volatile__ ("mov r2, %0\n" :: "r" (rstFlags));

  // disable watchdog
  MCUSR = 0;
  wdt_disable();

  // jump to app
  __asm__ __volatile__ (
    "clr r30\n"
    "clr r31\n"
    "ijmp\n"::
  );
}

// remove irq vector table
int main(void) __attribute__ ((OS_main)) __attribute__ ((section (".vectors")));
int main(void)
{
  // raw init
  __asm__ __volatile__ ("clr __zero_reg__");
#if defined(__AVR_ATmega8__) || defined (__AVR_ATmega32__) || defined (__AVR_ATmega16__)
  SP=RAMEND;  // This is done by hardware reset
#endif

  // get reset reason
  u08 rst_flag;
#ifdef MCUSR
  rst_flag = MCUSR;
  MCUSR = 0;
#elif defined(MCUCSR)
  rst_flag = MCUCSR;
  MCUCSR = 0;
#else
#error unknown avr
#endif

  // watchdog init
  wdt_enable(WDTO_500MS);

  // say hello
  uart_init();
  uart_send('P');

  // setup proto
  proto_init();
  uart_send('A');

  // check if bootloader command is set - if not enter app
  u08 cmd = proto_low_get_cmd();
  if(cmd != (PROTO_CMD_ACTION | PROTO_ACTION_BOOTLOADER)) {
    // check crc
    uart_send('B');
    u16 crc = pablo_check_rom_crc();
    if(crc == 0) {
      uart_send('L');
      // ensure that mach_tag matches in pablo footer
      u16 rom_mach_tag = pablo_get_mach_tag();
      if(rom_mach_tag == MACHTAG) {
        uart_send('O');
        // run app if valid
        run_app(rst_flag);
      }
    }
  }
  else {
    // reply to bootloader command
    uart_send('-');
    proto_low_action();
    proto_low_end();
  }

  // enter main loop
  uart_send(':');
  while(1) {
    wdt_reset();
    proto_handle();
  }
}

// msg i/o is used to transfer page data - channel is ignored in bootloader

u08 *proto_api_read_msg_prepare(u08 chan, u16 *size, u16 *extra)
{
  *size = SPM_PAGESIZE/2;
  uart_send('r');
  flash_read_page(page_addr, page_buf);
  return page_buf;
}

void proto_api_read_msg_done(u08 chan)
{
  uart_send('.');
}

u08 *proto_api_write_msg_prepare(u08 chan, u16 *max_size)
{
  *max_size = SPM_PAGESIZE/2;
  uart_send('w');
  return page_buf;
}

void proto_api_write_msg_done(u08 chan, u16 size, u16 extra)
{
  if(size == SPM_PAGESIZE/2) {
    uart_send('(');
    flash_program_page(page_addr, page_buf);
    uart_send(')');
    status = BOOT_STATUS_OK;
  } else {
    uart_send('?');
    status = BOOT_STATUS_INVALID_PAGE_SIZE;
  }
}
