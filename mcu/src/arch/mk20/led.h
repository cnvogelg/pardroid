#ifndef LED_H
#define LED_H

#include "arch.h"

extern void led_init(void);
extern void led_exit(void);
extern void led_set(int on);

INLINE void led_on(void)
{
  GPIOC_PSOR |= (uint32_t)(1 << 5);
}

INLINE void led_off(void)
{
  GPIOC_PCOR |= (uint32_t)(1 << 5);
}

#endif
