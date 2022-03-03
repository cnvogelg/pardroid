#ifndef HW_LED_H
#define HW_LED_H

#include "arch.h"

extern void hw_led_init(void);
extern void hw_led_exit(void);
extern void hw_led_set(int on);

INLINE void hw_led_on(void)
{
  GPIOC_PSOR |= (uint32_t)(1 << 5);
}

INLINE void hw_led_off(void)
{
  GPIOC_PCOR |= (uint32_t)(1 << 5);
}

#endif
