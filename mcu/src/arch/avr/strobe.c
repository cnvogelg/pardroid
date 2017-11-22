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
static volatile u08 exit_byte;
static u08 my_count;

ISR(PAR_STROBE_VECT)
{
  u08 data = strobe_low_get_data();

  // ack lo
  PAR_ACK_PORT &= ~PAR_ACK_MASK;

  // special exit byte?
  if(data == STROBE_MAGIC_BYTE_LO) {
    // lower BUSY
    PAR_BUSY_PORT &= ~PAR_BUSY_MASK;
  }
  else if(data == STROBE_MAGIC_BYTE_HI) {
    // high BUSY
    PAR_BUSY_PORT |= PAR_BUSY_MASK;
  }
  else if(data == STROBE_MAGIC_BYTE_EXIT) {
    exit_byte = 1;
  }
  // readable char?
  else if((data >= 32) && (data < 128)) {
    strobe_key <<= 8;
    strobe_key |= data;

    strobe_count++;
  } else {
    _delay_us(1);
  }

  // ack hi
  PAR_ACK_PORT |= PAR_ACK_MASK;
}

void strobe_init(void)
{
  strobe_low_init();

  strobe_key = 0;
  strobe_count = 0;
  my_count = 0;
  exit_byte = 0;

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
  // return with exit byte?
  if(exit_byte != 0) {
    *key = exit_byte;
    return 1;
  }

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
