#include "autoconf.h"
#include "types.h"

#include <avr/io.h>

#include "wiznet_low.h"
#include "wiznet_pins.h"
#include "timer.h"

#define reset_hi()     WIZNET_RESET_PORT |= WIZNET_RESET_MASK
#define reset_lo()     WIZNET_RESET_PORT &= ~WIZNET_RESET_MASK

void wiznet_low_reset(void)
{
  WIZNET_RESET_DDR |= WIZNET_RESET_MASK;
  reset_hi();
  reset_lo();
  timer_delay_us(2);
  reset_hi();
}
