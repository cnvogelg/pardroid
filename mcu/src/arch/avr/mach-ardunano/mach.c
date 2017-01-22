
#include <avr/interrupt.h>

void mach_init_hw(void)
{
  // disable watchdog
  cli();
  MCUSR &= ~_BV(WDRF);
  WDTCSR |= _BV(WDCE) | _BV(WDE);
  WDTCSR = 0;
  sei();
}
