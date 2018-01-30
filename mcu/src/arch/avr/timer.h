#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <stdint.h>

#include "types.h"

// unit of my timers
typedef u16 timer_ms_t;

extern volatile timer_ms_t  timer_ms;


extern void timer_init(void);

#define timer_delay(ms) _delay_ms(ms)
#define timer_delay_us(us) _delay_us(us)
#define timer_delay_1us()  _delay_loop_1(F_CPU / 1000000)

inline timer_ms_t timer_millis(void)
{
  timer_ms_t val;
  ATOMIC_BLOCK(ATOMIC_FORCEON)
  {
    val = timer_ms;
  }
  return val;
}

inline u08 timer_millis_timed_out(timer_ms_t start, u16 timeout)
{
  timer_ms_t cur = timer_millis();
  if(cur >= start) {
    return (cur - start) > timeout;
  } else {
    return (cur + (~start)) > timeout;
  }
}

#endif
