#include "kinetis.h"

// setup sys tick

static volatile uint32_t systick_millis_count;

void systick_isr(void)
{
    ++systick_millis_count;
}

static inline uint32_t millis(void)
{
    volatile uint32_t ret = systick_millis_count;
    return ret;
}

void timer_delay(uint32_t msec)
{
    uint32_t start = millis();
    while ((millis() - start) < msec);
}
