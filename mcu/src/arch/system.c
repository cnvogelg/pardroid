
#include <avr/wdt.h>
#include <avr/interrupt.h>

void system_init(void)
{
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
