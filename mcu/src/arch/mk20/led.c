#include "kinetis.h"

#include "led.h"

void led_init(void)
{
  // setup GPIO pin 13 aka PC5
  PORTC_PCR5 = (uint32_t)(1 << 8);
  GPIOC_PDDR |= (uint32_t)(1 << 5);
  GPIOC_PCOR |= (uint32_t)(1 << 5);
}

void led_exit(void)
{
  GPIOC_PDDR &= ~(uint32_t)(1 << 5);
  GPIOC_PCOR |= (uint32_t)(1 << 5);
}

void led_set(int on)
{
  if(on) {
    GPIOC_PSOR |= (uint32_t)(1 << 5);
  } else {
    GPIOC_PCOR |= (uint32_t)(1 << 5);
  }
}
