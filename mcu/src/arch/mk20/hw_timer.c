#include "kinetis.h"
#include "hw_timer.h"

// setup sys tick

volatile hw_timer_ms_t systick_millis_count;

void systick_isr(void)
{
    ++systick_millis_count;
}

void hw_timer_delay_ms(uint32_t msec)
{
    hw_timer_ms_t start = hw_timer_millis();
    while ((hw_timer_millis() - start) < msec);
}
