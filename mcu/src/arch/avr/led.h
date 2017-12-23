#ifndef LED_H
#define LED_H

#include <avr/io.h>

#include "led_pins.h"

extern void led_init(void);
extern void led_exit(void);
extern void led_set(u08 on);

static inline void led_on(void)
{
  LED_PORT |= LED_MASK;
}

static inline void led_off(void)
{
  LED_PORT &= ~LED_MASK;
}

#endif
