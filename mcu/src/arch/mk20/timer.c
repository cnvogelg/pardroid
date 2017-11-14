#include "kinetis.h"
#include "timer.h"

// setup sys tick

volatile uint32_t systick_millis_count;

void systick_isr(void)
{
    ++systick_millis_count;
}

void timer_delay(uint32_t msec)
{
    uint32_t start = timer_millis();
    while ((timer_millis() - start) < msec);
}
