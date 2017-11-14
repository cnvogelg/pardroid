#include "kinetis.h"

#include "led.h"

void led_init(void)
{
    // setup GPIO pin 13 aka PC5
    PORTC_PCR5 = (uint32_t)(1 << 8);
    GPIOC_PDDR |= (uint32_t)(1 << 5);
    GPIOC_PSOR |= (uint32_t)(1 << 5);
}

void led_set(int on)
{
    if(on) {
        GPIOC_PTOR |= (uint32_t)(1 << 5);
    } else {

    }
}
