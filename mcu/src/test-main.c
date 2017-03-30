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
#include "machtag.h"
#include "pend.h"

#include <util/delay.h>

#define MAX_TEST_MSG_SIZE 1024

static u16 test_reg;
static u16 test_size;
static u08 test_msg[MAX_TEST_MSG_SIZE];

// ro registers
static const u16 ro_version ROM_ATTR = VERSION_TAG;
static const u16 ro_machtag ROM_ATTR = MACHTAG;
static const u16 ro_rom_word ROM_ATTR = 1;
static const u08 ro_rom_byte ROM_ATTR = 2;
static u16 ro_ram_word = 3;
static u08 ro_ram_byte = 4;
REG_RO_TABLE_BEGIN
  /* proto regs */
  REG_RO_TABLE_ROM_W(ro_version),
  REG_RO_TABLE_ROM_W(ro_machtag),
  REG_RO_TABLE_RAM_W(pend_mask),
  REG_RO_TABLE_RAM_W(pend_total),
  /* user regs */
  REG_RO_TABLE_ROM_W(ro_rom_word),
  REG_RO_TABLE_ROM_B(ro_rom_byte),
  REG_RO_TABLE_RAM_W(ro_ram_word),
  REG_RO_TABLE_RAM_B(ro_ram_byte)
REG_RO_TABLE_END

// register ops

void proto_api_set_rw_reg(u08 reg,u16 val)
{
  if(reg == 0) {
    test_size = val;
  } else {
    test_reg = val;
  }
}

u16  proto_api_get_rw_reg(u08 reg)
{
  if(reg == 0) {
    return test_size;
  } else {
    return test_reg;
  }
}

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

  _delay_ms(300);

  DC('+');
  proto_init();
  pend_init();
  DC('-');

  test_reg = 0x4812;

  while(1) {
    system_wdt_reset();
    proto_handle();
    pend_handle();
#if 0
    _delay_ms(500);
    DC('.');
#endif
  }

  return 0;
}
