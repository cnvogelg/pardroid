#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "uart.h"
#include "uartutil.h"
#include "proto.h"
#include "debug.h"
#include "mach.h"
#include "pablo.h"
#include "reg_ro.h"
#include "machtag.h"

#include <util/delay.h>

#define MAX_TEST_MSG_SIZE 1024

static u16 test_reg;
static u16 test_size;
static u08 test_msg[MAX_TEST_MSG_SIZE];

// ro registers
const u16 val_one ROM_ATTR = 1;
const u16 val_two ROM_ATTR = 2;
REG_RO_TABLE_BEGIN
  REG_RO_TABLE_ENTRY(val_one),
  REG_RO_TABLE_ENTRY(val_two)
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
  machtag_decode(mt, &arch, &mcu, &mach);
  uart_send_pstring(arch);
  uart_send('-');
  uart_send_pstring(mcu);
  uart_send('-');
  uart_send_pstring(mach);
  uart_send_crlf();
}

int main(void)
{
  mach_init_hw();

  uart_init();
  uart_send_pstring(PSTR("parbox-test!"));
  uart_send_crlf();

  rom_info();

  _delay_ms(300);

  DC('+');
  proto_init();
  DC('-');

  test_reg = 0x4812;

  while(1) {
    mach_wdt_reset();
    proto_handle();
#if 0
    _delay_ms(500);
    DC('.');
#endif
  }

  return 0;
}
