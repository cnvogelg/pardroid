#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "uart.h"
#include "uartutil.h"
#include "proto.h"
#include "proto_shared.h"
#include "debug.h"
#include "system.h"
#include "pablo.h"
#include "reg.h"
#include "reg_def.h"
#include "action.h"
#include "machtag.h"
#include "handler.h"
#include "status.h"

#include <util/delay.h>

#define MAX_TEST_MSG_SIZE 1024

static u08 test_msg[MAX_TEST_MSG_SIZE];

static void sim_set_pending(void)
{
  DS("set_pending"); DNL;
  status_set_pending(0);
}

static void sim_clear_pending(void)
{
  DS("clear_pending"); DNL;
  status_clear_pending();
}

// ----- actions -----
ACTION_TABLE_BEGIN
  ACTION_PROTO_DEFAULTS
  ACTION_TABLE_FUNC(sim_set_pending),
  ACTION_TABLE_FUNC(sim_clear_pending)
ACTION_TABLE_END

// ----- ro registers -----
// read-only test values
static const u16 ro_rom_word ROM_ATTR = 1;
static const u08 ro_rom_byte ROM_ATTR = 2;
static u16 ro_ram_word = 3;
static u08 ro_ram_byte = 4;
static u16 ro_func(void) {
  return 5;
}

// read/write test values
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

REG_PROTO_APPID(PROTO_FWID_TEST)
REG_TABLE_BEGIN
  REG_TABLE_DEFAULTS
  /* user read-only regs */
  REG_TABLE_RO_ROM_W(ro_rom_word),      // user+0
  REG_TABLE_RO_ROM_B(ro_rom_byte),      // user+1
  REG_TABLE_RO_RAM_W(ro_ram_word),      // user+2
  REG_TABLE_RO_RAM_B(ro_ram_byte),      // user+3
  REG_TABLE_RO_FUNC(ro_func),           // user+4
  /* user read-write regs */
  REG_TABLE_RW_FUNC(get_test_size, set_test_size), // user+5
  REG_TABLE_RW_RAM_W(test_word),                   // user+6
  REG_TABLE_RW_RAM_B(test_byte)                    // user+7
REG_TABLE_END

// my handler

void my_init(void)
{
}

void my_work(void)
{
}

u08 *my_read_msg_prepare(u16 *size)
{
  *size = test_size;
  return test_msg;
}

void my_read_msg_done(void)
{
}

u08 *my_write_msg_prepare(u16 *max_size)
{
  *max_size = MAX_TEST_MSG_SIZE >> 1;
  return test_msg;
}

void my_write_msg_done(u16 size)
{
  test_size = size;
}

// handler

HANDLER_TABLE_BEGIN
  HANDLER_ENTRY(my_init, my_work, my_read_msg_prepare, my_read_msg_done, my_write_msg_prepare, my_write_msg_done)
HANDLER_TABLE_END

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
  handler_init();
  DC('-'); DNL;

  while(1) {
    system_wdt_reset();
    proto_handle();
    status_handle();
    handler_work();
  }

  return 0;
}
