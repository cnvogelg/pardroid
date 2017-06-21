#include <avr/wdt.h>
#include <avr/boot.h>
#include <avr/pgmspace.h>

#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "bootloader.h"
#include "uart.h"
#include "uartutil.h"
#include "proto.h"
#include "proto_low.h"
#include "flash.h"
#include "pablo.h"
#include "reg.h"
#include "action.h"
#include "func.h"
#include "machtag.h"

static u08 status;
static u16 page_addr;
static u08 page_buf[SPM_PAGESIZE];

// action
ACTION_TABLE_BEGIN
  ACTION_PROTO_BOOTLOADER
ACTION_TABLE_END

// ----- functions -----
FUNC_TABLE_BEGIN
  FUNC_PROTO_BOOTLOADER
FUNC_TABLE_END

// ro registers
static const u16 ro_version ROM_ATTR = VERSION_TAG;
static const u16 ro_mach_tag ROM_ATTR = MACHTAG;
static const u16 ro_page_size ROM_ATTR = SPM_PAGESIZE;
static const u16 ro_rom_size ROM_ATTR = CONFIG_MAX_ROM;
REG_TABLE_BEGIN
  REG_TABLE_RO_ROM_W(ro_version),               /* 0: bl version */
  REG_TABLE_RO_ROM_W(ro_mach_tag),              /* 1: bl mach tag */
  REG_TABLE_RO_ROM_W(ro_page_size),             /* 2: page size */
  REG_TABLE_RO_ROM_W(ro_rom_size),              /* 3: rom size */
  REG_TABLE_RO_ROM_W_PTR(CONFIG_MAX_ROM-2),     /* 4: rom crc */
  REG_TABLE_RO_ROM_W_PTR(CONFIG_MAX_ROM-4),     /* 5: rom mach tag */
  REG_TABLE_RO_ROM_W_PTR(CONFIG_MAX_ROM-6),     /* 6: rom version */
  REG_TABLE_RW_RAM_W(page_addr)                 /* 7: (rw) page addr */
REG_TABLE_END

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
  proto_init(PROTO_STATUS_BOOTLOADER);
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
        uart_send_crlf();
        // run app if valid
        run_app(rst_flag);
      }
    }
  }
  else {
    // reply to bootloader command
    uart_send('-');
    proto_low_action();
    proto_low_end(PROTO_STATUS_BOOTLOADER);
  }

  // enter main loop
  uart_send(':');
  while(1) {
    wdt_reset();
    proto_handle();
  }
}

// msg i/o is used to transfer page data - channel is ignored in bootloader

#define PAGE_WORDS (SPM_PAGESIZE >> 1)

u08 *proto_api_read_msg_prepare(u08 chan, u16 *size)
{
  *size = PAGE_WORDS;
  uart_send('r');
  flash_read_page(page_addr, page_buf);
  return page_buf;
}

void proto_api_read_msg_done(u08 chan)
{
  uart_send('.');
  uart_send_crlf();
}

u08 *proto_api_write_msg_prepare(u08 chan, u16 *max_size)
{
  *max_size = PAGE_WORDS;
  uart_send('w');
  return page_buf;
}

void proto_api_write_msg_done(u08 chan, u16 size)
{
  if(size == PAGE_WORDS) {
    uart_send('(');
    flash_program_page(page_addr, page_buf);
    uart_send(')');
    status = BOOT_STATUS_OK;
  } else {
    uart_send('?');
    status = BOOT_STATUS_INVALID_PAGE_SIZE;
  }
  uart_send_crlf();
}

u08 proto_api_read_is_pending(void)
{
  return 0;
}

u08 proto_api_get_end_status(void)
{
  return PROTO_STATUS_BOOTLOADER;
}
