#ifndef LED_H
#define LED_H

#include <avr/io.h>

#include "arch.h"
#include "led_pins.h"

extern void led_init(void);
extern void led_exit(void);
extern void led_set(u08 on);

FORCE_INLINE void led_on(void)
{
  LED_PORT |= LED_MASK;
}

FORCE_INLINE void led_off(void)
{
  LED_PORT &= ~LED_MASK;
}

#endif
