#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <util/delay_basic.h>

#include "arch.h"
#include "autoconf.h"
#include "types.h"
#include "strobe.h"
#include "pario_pins.h"
#include "system.h"

#include "uart.h"

#define  ITER         (F_CPU / 1000000)
#define  DELAY_1US()  _delay_loop_1(ITER)

#define  ack_lo  PAR_ACK_PORT &= ~PAR_ACK_MASK
#define  ack_hi  PAR_ACK_PORT |= PAR_ACK_MASK
#define  busy_lo  PAR_BUSY_PORT &= ~PAR_BUSY_MASK
#define  busy_hi  PAR_BUSY_PORT |= PAR_BUSY_MASK
#define  is_busy()  ((PAR_BUSY_PIN & PAR_BUSY_MASK) == PAR_BUSY_MASK)

// strobe_low_asm functions
extern void strobe_low_init(void);
extern void strobe_low_exit(void);
extern u08  strobe_low_get_data(void);
extern void strobe_low_begin_send(u08 data);
extern void strobe_low_end_send(void);
extern void strobe_low_set_data(u08 data);

static volatile u08 state;
static u08 strobe_count;
static volatile u32 strobe_key;

static rom_pchar send_data;
static u16 send_size;

typedef void (*strobe_func_t)(void);

static strobe_func_t strobe_func;

ISR(PAR_STROBE_VECT)
{
  // dispatch to read/write func
  strobe_func();
}

static void strobe_read_func(void)
{
  u08 data = strobe_low_get_data();

  // readable char?
  if((data >= 32) && (data < 128)) {
    strobe_key <<= 8;
    strobe_key |= data;
    strobe_count++;
    if(strobe_count == 4) {
      state = 1;
      strobe_count = 0;
    }
  }

  // pulse ack
  ack_lo;
  DELAY_1US();
  ack_hi;
}

void strobe_init(void)
{
  strobe_low_init();

  strobe_key = 0;
  strobe_count = 0;
  state = 0;

  strobe_func = strobe_read_func;

  // setup int irq
  cli();
  // falling edge on INTx -> strobe detect
  PAR_STROBE_EICR = _BV(PAR_STROBE_ISC);
#ifdef GICR
  GICR = _BV(PAR_STROBE_INT);
#else
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
  u08 exit = 0;
  ATOMIC_BLOCK(ATOMIC_FORCEON)
  {
    if(state != 0) {
      *key = strobe_key;
      state = 0;
      exit = 1;
    }
  }
  return exit;
}

u08 strobe_get_data(void)
{
  return strobe_low_get_data();
}

// ---------- send stuff ----------

static void strobe_write_func(void)
{
  // nothing more to send...
  if(send_size == 0) {
    strobe_low_set_data(0);
    state = STROBE_FLAG_GOT_STROBE | STROBE_FLAG_ALL_SENT;
  } else {
    state = STROBE_FLAG_GOT_STROBE;

    // setup next byte
    u08 data = pgm_read_byte(send_data);
    strobe_low_set_data(data);
    send_data++;
    send_size--;
  }

  // 0xc4 is AmigaDOS buffer size for type command
  strobe_count++;
  if(strobe_count == 0xc4) {
    strobe_count = 0;
    // suppress ack if ended
    if(send_size == 0) {
      state |= STROBE_FLAG_BUFFER_FILLED;
      return;
    }
  }

  // pulse ack
  ack_lo;
  DELAY_1US();
  ack_hi;
}

void strobe_send_begin(rom_pchar data, u16 size)
{
  // keep send range
  send_data = data;
  send_size = size;

  // setup first byte
  u08 val = pgm_read_byte(send_data);
  send_data++;
  send_size--;
  strobe_low_begin_send(val);

  state = STROBE_FLAG_NONE;
  strobe_count = 0;
  strobe_func = strobe_write_func;
}

u08 strobe_read_flag(void)
{
  // read busy
  u08 res = is_busy() ? STROBE_FLAG_IS_BUSY : STROBE_FLAG_NONE;
  ATOMIC_BLOCK(ATOMIC_FORCEON)
  {
    res |= state;
    state = STROBE_FLAG_NONE;
  }
  return res;
}

void strobe_pulse_ack(void)
{
  // pulse ack
  ack_lo;
  DELAY_1US();
  ack_hi;
}

void strobe_send_end(void)
{
  strobe_low_end_send();

  strobe_func = strobe_read_func;

  state = 0;
  strobe_count = 0;
}
