#include "autoconf.h"
#include "types.h"

#include "led.h"

void led_init(void)
{
  LED_DDR  |= LED_MASK;
  LED_PORT &= ~LED_MASK;
}

void led_exit(void)
{
  LED_DDR &= ~LED_MASK;
  LED_PORT &= ~LED_MASK;
}

void led_set(u08 on)
{
  if(on) {
    LED_PORT |= LED_MASK;
  } else {
    LED_PORT &= ~LED_MASK;
  }
}
