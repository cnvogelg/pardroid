#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "uart.h"
#include "uartutil.h"
#include "rominfo.h"
#include "fwid.h"
#include "proto.h"
#include "reg.h"
#include "system.h"
#include "timer.h"

#include "status.h"
#include "base_reg.h"
#include "pamela.h"

// max size of message buffer
#define MAX_BUFFER_SIZE 1024

static u08 buffer[MAX_BUFFER_SIZE];
static u16 extra_val;
static u08 test_mode;
static u16 test_count;
static u16 busy_delay;

#define TEST_MODE_NORMAL 0
#define TEST_MODE_ECHO   1

// sim functions
static void sim_pending(u16 *valp, u08 mode)
{
  if(mode == REG_MODE_WRITE) {
    u08 v = (u08)(*valp & 0xff);
    u08 cmd = v & 0x80;
    u08 chn = v & 0x7f;
    if(cmd == 0x00) {
      DS("sim:p-"); DB(chn); DNL;
      status_clear_pending(chn);
    } else {
      DS("sim:p+"); DB(chn); DNL;
      status_set_pending(chn);
    }
  }
}

static void sim_event(u16 *valp, u08 mode)
{
  if(mode == REG_MODE_WRITE) {
    u08 chn = *valp & 0xff;
    DS("sim:e"); DB(chn); DNL;
    status_set_event(chn);
  }
}

static void set_test_mode(u16 *valp, u08 mode)
{
  if(mode == REG_MODE_WRITE) {
    test_mode = *valp & 0xff;
    DS("test_mode:"); DB(test_mode); DNL;
  }
}

static void set_test_count(u16 *valp, u08 mode)
{
  if(mode == REG_MODE_WRITE) {
    test_count = *valp;
    DS("test_count:"); DW(test_count); DNL;
  }
}

static void sim_delay(u16 *valp, u08 mode)
{
  if(mode == REG_MODE_WRITE) {
    busy_delay = *valp;
    DS("busy_delay:"); DW(busy_delay); DNL;
  }
}

// ----- ro registers -----
// read-only test values
static const u16 ro_rom_word ROM_ATTR = 1;
static u16 ro_ram_word = 3;
static void func_max_size(u16 *val, u08 mode) {
  *val = MAX_BUFFER_SIZE;
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
BASE_REG_APPID(FWID_TEST_PAMELA)

// registers
REG_TABLE_BEGIN(test)
  /* user read-only regs */
  REG_TABLE_RO_ROM_W(ro_rom_word),      // user+0
  REG_TABLE_RO_RAM_W(ro_ram_word),      // user+1
  REG_TABLE_RO_FUNC(func_max_size),     // user+2
  /* user read-write regs */
  REG_TABLE_RW_FUNC(func_test_size),    // user+3
  REG_TABLE_RW_RAM_W(test_word),        // user+4
  REG_TABLE_RW_FUNC(sim_pending),       // user+5
  REG_TABLE_RW_FUNC(sim_event),         // user+6
  REG_TABLE_RW_FUNC(set_test_mode),     // user+7
  REG_TABLE_RW_FUNC(set_test_count),    // user+8
  REG_TABLE_RW_FUNC(sim_delay)          // user+9
REG_TABLE_END(test, PROTO_REGOFFSET_USER, REG_TABLE_REF(base))
REG_TABLE_SETUP(test)

// long registers

static u32 regl_val;

void func_api_set_regl(u08 slot, u32 val)
{
  regl_val = val;
}

u32 func_api_get_regl(u08 slot)
{
  return regl_val;
}

// message buffer handling

u08 *proto_api_read_msg_prepare(u08 chn, u16 *ret_size, u16 *extra)
{
  DS("[R#"); DB(chn); DC('+'); DW(test_size); DC('%'); DW(extra_val); DC(']');
  *ret_size = test_size;
  *extra = extra_val;
  return buffer;
}

void proto_api_read_msg_done(u08 chn, u08 status)
{
  DS("[r#"); DB(chn); DC(':'); DB(status); DC(']'); DNL;

  if(test_mode == TEST_MODE_ECHO) {
    status_clear_pending(chn);
  }
}

u08 *proto_api_write_msg_prepare(u08 chn, u16 *max_size)
{
  *max_size = MAX_BUFFER_SIZE;
  DS("[W#"); DB(chn); DC(':'); DW(*max_size); DC(']');
  return buffer;
}

void proto_api_write_msg_done(u08 chn, u16 size, u16 extra)
{
  DS("[w#"); DB(chn); DC(':'); DW(size);DC('%'); DW(extra); DC(']'); DNL;
  test_size = size;
  extra_val = extra;

  if(test_mode == TEST_MODE_ECHO) {
    status_set_pending(chn);
  }
}

static void sim_busy(void)
{
  uart_send('{');
  status_set_busy();
  timer_ms_t t0 = timer_millis();
  timer_ms_t t1 = timer_millis();
  while(1) {
    /* done? */
    if(timer_millis_timed_out(t0, busy_delay)) {
      break;
    }
    /* some output */
    if(timer_millis_timed_out(t1, 100)) {
      uart_send('.');
      t1 = timer_millis();
    }
    /* keep watchdog happy */
    system_wdt_reset();
  }
  status_clear_busy();
  uart_send('}');
  uart_send_crlf();
}

int main(void)
{
  system_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-pamela!"));
  uart_send_crlf();

  rom_info();

  pamela_init();

  while(1) {
    system_wdt_reset();
    pamela_handle();

    if(busy_delay != 0) {
      sim_busy();
      busy_delay = 0;
    }
  }

  return 0;
}
