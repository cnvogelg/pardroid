#ifndef TIMER_H
#define TIMER_H

#include <stdint.h>

extern void timer_delay(uint32_t ms);

extern volatile uint32_t systick_millis_count;

static inline uint32_t timer_millis(void)
{
    volatile uint32_t ret = systick_millis_count;
    return ret;
}

#endif
