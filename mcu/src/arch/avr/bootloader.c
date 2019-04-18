#include <avr/wdt.h>
#include <avr/boot.h>

#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "bootbase.h"

static u08 page_buf[SPM_PAGESIZE];
static u08 rst_flag;

// from optiboot
static void run_app(void) __attribute__ ((naked));
static void run_app(void)
{
  // store reset reason
  __asm__ __volatile__ ("mov r2, %0\n" :: "r" (rst_flag));

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

void boot_wdt_reset(void)
{
  wdt_reset();
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

  // check for app
  u08 ret = bootbase_init(SPM_PAGESIZE, page_buf);
  if(ret == BOOTBASE_RET_RUN_APP) {
    run_app();
  }
}
