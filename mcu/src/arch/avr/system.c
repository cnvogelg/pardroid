
#include <avr/wdt.h>
#include <avr/interrupt.h>

#include "autoconf.h"

void system_init(void)
{
#ifdef MACH_TEENSY20
  // set teensy clock to 16 MHz
#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define CPU_16MHz       0x00
  CPU_PRESCALE(CPU_16MHz);
#endif

  // default watchdog: 500ms
  cli();
  wdt_enable(WDTO_500MS);
  sei();
}

void system_sys_reset(void)
{
  wdt_enable(WDTO_15MS);
  while(1) { /* wait for the end */ }
}

void system_wdt_reset(void)
{
  wdt_reset();
}

