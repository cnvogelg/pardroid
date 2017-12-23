#ifndef LED_H
#define LED_H

extern void led_init(void);
extern void led_exit(void);
extern void led_set(int on);

static inline void led_on(void)
{
  GPIOC_PSOR |= (uint32_t)(1 << 5);
}

static inline void led_off(void)
{
  GPIOC_PCOR |= (uint32_t)(1 << 5);
}

#endif
