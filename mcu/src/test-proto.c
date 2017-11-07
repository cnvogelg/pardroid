#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "uart.h"
#include "uartutil.h"
#include "rominfo.h"
#include "proto.h"
#include "reg.h"
#include "system.h"
#include "status.h"
#include "base_reg.h"

// max size of message buffer
#define MAX_BUFFER_SIZE 1024

static u08 buffer[MAX_BUFFER_SIZE];


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
static u16 test_size = MAX_BUFFER_SIZE;
static void func_test_size(u16 *val, u08 mode)
{
  if(mode == REG_MODE_WRITE) {
    if(*val <= MAX_BUFFER_SIZE) {
      test_size = *val;
    }
  } else {
    *val = test_size;
  }
}

// define my app id
BASE_REG_APPID(PROTO_FWID_TEST_PROTO)

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
REG_TABLE_END(test, PROTO_REGOFFSET_USER, REG_TABLE_REF(base))
REG_TABLE_SETUP(test)

// message buffer handling

u08 *proto_api_read_msg_prepare(u08 chn, u16 *ret_size)
{
  DS("[R#"); DB(chn); DC('+'); DW(test_size);
  *ret_size = test_size;
  return buffer;
}

void proto_api_read_msg_done(u08 chn, u08 status)
{
  DC(']'); DNL;
}

u08 *proto_api_write_msg_prepare(u08 chn, u16 *max_size)
{
  DS("[W#"); DB(chn); DC(':'); DW(test_size);
  *max_size = test_size;
  return buffer;
}

void proto_api_write_msg_done(u08 chn, u16 size)
{
  DC(':'); DW(size); DC(']'); DNL;
}


int main(void)
{
  system_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-proto!"));
  uart_send_crlf();

  rom_info();

  DC('+');
  proto_init(PROTO_STATUS_INIT);
  status_init();
  DC('-'); DNL;

  while(1) {
    system_wdt_reset();
    proto_handle();
    status_handle();
  }

  return 0;
}
