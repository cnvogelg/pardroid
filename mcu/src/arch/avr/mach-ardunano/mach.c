
#include <avr/wdt.h>
#include <avr/interrupt.h>

void mach_init_hw(void)
{
  // disable watchdog
  cli();
  wdt_enable(WDTO_500MS);
  sei();
}

void mach_sys_reset(void)
{
  wdt_enable(WDTO_15MS);
  while(1) { /* wait for the end */ }
}
