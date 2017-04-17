#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "uart.h"
#include "uartutil.h"
#include "proto.h"
#include "debug.h"
#include "system.h"
#include "pablo.h"
#include "reg_ro.h"
#include "reg_ro_def.h"
#include "reg_rw.h"
#include "reg_rw_def.h"
#include "machtag.h"
#include "pend.h"

#include <util/delay.h>

#define MAX_TEST_MSG_SIZE 1024

static u08 test_msg[MAX_TEST_MSG_SIZE];

// ----- ro registers -----
// test values
static const u16 ro_rom_word ROM_ATTR = 1;
static const u08 ro_rom_byte ROM_ATTR = 2;
static u16 ro_ram_word = 3;
static u08 ro_ram_byte = 4;
static u16 ro_func(void) {
  return 5;
}

REG_RO_PROTO_APPID(PROTO_FWID_TEST)
REG_RO_TABLE_BEGIN
  REG_RO_PROTO_DEFAULTS
  /* user regs */
  REG_RO_TABLE_ROM_W(ro_rom_word),
  REG_RO_TABLE_ROM_B(ro_rom_byte),
  REG_RO_TABLE_RAM_W(ro_ram_word),
  REG_RO_TABLE_RAM_B(ro_ram_byte),
  REG_RO_TABLE_FUNC(ro_func)
REG_RO_TABLE_END

// ----- rw registers -----
// test values
static u16 test_word = 0x4812;
static u08 test_byte = 0x42;
static u16 test_size;
static void set_test_size(u16 val)
{
  if(val <= (MAX_TEST_MSG_SIZE / 2)) {
    test_size = val;
  }
}
static u16 get_test_size(void)
{
  return test_size;
}

REG_RW_TABLE_BEGIN
  REG_RW_PROTO_DEFAULTS
  /* user regs */
  REG_RW_TABLE_FUNC(get_test_size, set_test_size),
  REG_RW_TABLE_RAM_W(test_word),
  REG_RW_TABLE_RAM_B(test_byte)
REG_RW_TABLE_END

// msg ops

u08 *proto_api_prepare_read_msg(u08 chan, u16 *size)
{
  *size = test_size;
  return test_msg;
}

void proto_api_done_read_msg(u08 chan)
{
}

u08 *proto_api_prepare_write_msg(u08 chan, u16 *max_size)
{
  *max_size = MAX_TEST_MSG_SIZE >> 1;
  return test_msg;
}

void proto_api_done_write_msg(u08 chan, u16 size)
{
  test_size = size;
}

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
  proto_init();
  pend_init();
  DC('-');

  while(1) {
    system_wdt_reset();
    proto_handle();
    pend_handle();
  }

  return 0;
}
