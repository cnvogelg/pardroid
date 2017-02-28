#include <avr/wdt.h>

#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "uart.h"
#include "proto.h"
#include "proto_low.h"

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
#elif MCUCSR
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
  if(cmd != CMD_BOOTLOADER) {
    uart_send('!');
    run_app(rst_flag);
  }

  // reply to bootloader command
  proto_low_no_value();
  uart_send('B');

  // enter main loop
  while(1) {
    wdt_reset();
    proto_handle();
  }
}

void proto_api_set_reg(u08 reg,u16 val)
{
}

u16  proto_api_get_reg(u08 reg)
{
  return 0;
}

u16  proto_api_get_const(u08 num)
{
  return 0x4711;
}

u08 *proto_api_get_read_msg(u16 *size)
{
  *size = 0;
  return 0;
}

u08 *proto_api_get_write_msg(u16 *max_size)
{
  *max_size = 0;
  return 0;
}

void proto_api_set_write_msg_size(u16 size)
{
}
