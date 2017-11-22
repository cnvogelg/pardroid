#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "autoconf.h"
#include "types.h"
#include "strobe.h"
#include "pario_pins.h"
#include "system.h"

extern void strobe_low_init(void);
extern void strobe_low_exit(void);
extern u08  strobe_low_get_data(void);

static volatile u08 strobe_count;
static volatile u32 strobe_key;
static u08 my_count;

ISR(PAR_STROBE_VECT)
{
  u08 data = strobe_low_get_data();
  if(data < 32) {
    return;
  }

  // ack lo
  PAR_ACK_PORT &= ~PAR_ACK_MASK;
  _delay_us(3);
  PAR_ACK_PORT |= PAR_ACK_MASK;

  strobe_key <<= 8;
  strobe_key |= data;
  strobe_count++;
}

void strobe_init(void)
{
  strobe_low_init();

  strobe_key = 0;
  strobe_count = 0;
  my_count = 0;

  // setup int irq
  cli();
  // falling edge on INTx -> strobe detect
#ifdef GICR
  MCUCR = _BV(PAR_STROBE_ISC);
  GICR = _BV(PAR_STROBE_INT);
#else
  EICRA = _BV(PAR_STROBE_ISC);
  EIMSK = _BV(PAR_STROBE_INT);
#endif
  sei();
}

void strobe_exit(void)
{
  strobe_low_exit();

  cli();
#ifdef GICR
  GICR = 0;
#else
  EIMSK = 0;
#endif
  sei();
}

u08 strobe_get_key(u32 *key)
{
  // no data
  if(my_count == strobe_count) {
    return 0;
  }
  my_count = strobe_count;
  // too short
  if(my_count < 4) {
    return 0;
  }
  cli();
  *key = strobe_key;
  sei();
  return 1;
}
