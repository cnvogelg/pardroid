#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "uart.h"
#include "uartutil.h"
#include "proto.h"
#include "proto_shared.h"
#include "debug.h"
#include "system.h"
#include "pablo.h"
#include "reg.h"
#include "base_reg.h"
#include "action.h"
#include "func.h"
#include "machtag.h"
#include "status.h"
#include "buffer.h"
#include "handler.h"
#include "handler_reg.h"
#include "hnd_echo.h"
#include "hnd_null.h"
#include "driver.h"
#include "drv_null.h"

#include <util/delay.h>

// sim functions
static void sim_pending(u16 *valp, u08 mode)
{
  if(mode == REG_MODE_WRITE) {
    u08 v = *valp & 0xff;
    if(v == 0xff) {
      DS("sim:p-"); DNL;
      status_clear_pending();
    } else {
      DS("sim:p+"); DB(v); DNL;
      status_set_pending(v);
    }
  }
}

static void sim_error(u16 *valp, u08 mode)
{
  if(mode == REG_MODE_WRITE) {
    u08 e = *valp & 0xff;
    DS("sim:e"); DB(e); DNL;
    status_set_error(e);
  }
}

// ----- ro registers -----
// read-only test values
static const u16 ro_rom_word ROM_ATTR = 1;
static u16 ro_ram_word = 3;
static void ro_func(u16 *val, u08 mode) {
  *val = 5;
}

// read/write test values
static u16 test_word = 0x4812;
static u16 test_size;
static void func_test_size(u16 *val, u08 mode)
{
  if(mode == REG_MODE_WRITE) {
    if(*val <= 0x1234) {
      test_size = *val;
    }
  } else {
    *val = test_size;
  }
}

// define my app id
BASE_REG_APPID(PROTO_FWID_TEST)

// registers
REG_TABLE_BEGIN(test)
  /* user read-only regs */
  REG_TABLE_RO_ROM_W(ro_rom_word),      // user+0
  REG_TABLE_RO_RAM_W(ro_ram_word),      // user+1
  REG_TABLE_RO_FUNC(ro_func),           // user+2
  /* user read-write regs */
  REG_TABLE_RW_FUNC(func_test_size),    // user+3
  REG_TABLE_RW_RAM_W(test_word),        // user+4
  REG_TABLE_RW_FUNC(sim_pending),       // user+5
  REG_TABLE_RW_FUNC(sim_error)          // user+6
REG_TABLE_END(test, PROTO_REGOFFSET_USER, REG_TABLE_REF(handler))
REG_TABLE_SETUP(test)

// handler
HANDLER_TABLE_BEGIN
  HANDLER_TABLE_ENTRY(echo),
  HANDLER_TABLE_ENTRY(echo),
  HANDLER_TABLE_ENTRY(null)
HANDLER_TABLE_END

// driver
DRIVER_TABLE_BEGIN
  DRIVER_TABLE_ENTRY(null)
DRIVER_TABLE_END

static void rom_info(void)
{
  // show pablo infos
  u16 crc = pablo_check_rom_crc();
  u16 mt  = pablo_get_mach_tag();
  u16 ver = pablo_get_rom_version();
  uart_send_pstring(PSTR("crc="));
  uart_send_hex_word(crc);
  uart_send_pstring(PSTR(" machtag="));
  uart_send_hex_word(mt);
  uart_send_pstring(PSTR(" version="));
  uart_send_hex_word(ver);
  uart_send_crlf();

  // decode machtag
  rom_pchar arch,mcu,mach;
  u08 extra;
  machtag_decode(mt, &arch, &mcu, &mach, &extra);
  uart_send_pstring(arch);
  uart_send('-');
  uart_send_pstring(mcu);
  uart_send('-');
  uart_send_pstring(mach);
  uart_send('-');
  uart_send_hex_byte(extra);
  uart_send_crlf();
}

int main(void)
{
  system_init();

  uart_init();
  uart_send_pstring(PSTR("parbox-test!"));
  uart_send_crlf();

  rom_info();

  DC('+');
  proto_init(PROTO_STATUS_INIT);
  status_init();
  buffer_init();
  DRIVER_INIT();
  HANDLER_INIT();
  DC('-'); DNL;

  while(1) {
    system_wdt_reset();
    proto_handle();
    status_handle();
    DRIVER_WORK();
    HANDLER_WORK();
  }

  return 0;
}
