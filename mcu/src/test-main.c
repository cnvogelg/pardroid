#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "uart.h"
#include "uartutil.h"
#include "proto.h"
#include "debug.h"
#include "mach.h"

#include <util/delay.h>

#define MAX_TEST_MSG_SIZE 1024

static u16 test_reg;
static u16 test_size;
static u08 test_msg[MAX_TEST_MSG_SIZE];

u16 bla(u16 a, u16 b)
{
  if(a > b) {
    return a+b;
  } else {
    return a+1;
  }
}

void proto_api_set_reg(u08 reg,u16 val)
{
  if(reg == 0) {
    test_size = val;
  } else {
    test_reg = val;
  }
}

u16  proto_api_get_reg(u08 reg)
{
  if(reg == 0) {
    return test_size;
  } else {
    return test_reg;
  }
}

u16  proto_api_get_const(u08 num)
{
  return 0x4711;
}

u08 *proto_api_get_read_msg(u16 *size)
{
  *size = test_size;
  return test_msg;
}

u08 *proto_api_get_write_msg(u16 *max_size)
{
  *max_size = MAX_TEST_MSG_SIZE >> 1;
  return test_msg;
}

void proto_api_set_write_msg_size(u16 size)
{
  test_size = size;
}

int main(void)
{
  mach_init_hw();

  uart_init();
  uart_send_pstring(PSTR("parbox-test!\n"));
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
