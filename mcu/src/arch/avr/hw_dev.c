#include "autoconf.h"
#include "types.h"

#include <avr/io.h>

#include "hw_dev.h"
#include "dev_pins.h"
#include "hw_timer.h"

#define reset_hi()     DEV_RESET_PORT |= DEV_RESET_MASK
#define reset_lo()     DEV_RESET_PORT &= ~DEV_RESET_MASK

void hw_dev_init(void)
{
  DEV_RESET_DDR |= DEV_RESET_MASK;
  reset_hi();
}

void hw_dev_reset(void)
{
  reset_lo();
  hw_timer_delay_us(2);
  reset_hi();
}
