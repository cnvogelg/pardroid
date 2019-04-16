#include <stdint.h>
#include "kinetis.h"
#include "arch.h"

#include "autoconf.h"
#include "types.h"
#include "strobe.h"
#include "knok.h"
#include "pario_pins.h"
#include "timer.h"

static volatile u08 state;
static u08 strobe_count;
static volatile u32 strobe_key;

static rom_pchar send_data;
static u16 send_size;

typedef void (*strobe_func_t)(void);

static strobe_func_t strobe_func;

static void my_portc_isr(void)
{
  uint32_t isfr = PORTC_ISFR;
  PORTC_ISFR = isfr;
  if(isfr & STROBE_MASK) {
    strobe_func();
  }
}

static void strobe_read_func(void)
{
  u08 data = pario_get_data();

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
  pario_ack_lo();
  timer_delay_1us();
  pario_ack_hi();
}

static void attachInterruptVector(enum IRQ_NUMBER_t irq, void (*function)(void))
{
  _VectorsRam[irq + 16] = function;
}

void strobe_init_port(void)
{
  pario_init();
}

void strobe_init_irq(void)
{
  strobe_key = 0;
  strobe_count = 0;
  state = 0;

  strobe_func = strobe_read_func;

  /* attach falling edge irq to strobe
    /STROBE        1               23               PC2        in
  */
#define STROBE_PCR  PORTC_PCR2
  // falling edge
  uint32_t mask = (0x0a << 16) | 0x01000000;

  // setup int irq
  __disable_irq();

  uint32_t cfg = STROBE_PCR;
  cfg &= ~0x000F0000;   // disable any previous interrupt
  STROBE_PCR = cfg;
  cfg |= mask;
  STROBE_PCR = cfg;      // enable the new interrupt

  attachInterruptVector(IRQ_PORTC, my_portc_isr);

  NVIC_ENABLE_IRQ(IRQ_PORTC);

  __enable_irq();
}

void strobe_exit(void)
{
  // setup int irq
  __disable_irq();

  uint32_t cfg = STROBE_PCR;
  cfg &= ~0x000F0000;   // disable any previous interrupt
  STROBE_PCR = cfg;

  __enable_irq();
}

u08 strobe_get_key(u32 *key)
{
  // return with exit byte?
  u08 exit = 0;

  __disable_irq();

  if(state != 0) {
    *key = strobe_key;
    state = 0;
    exit = 1;
  }

  __enable_irq();

  return exit;
}

u08 strobe_get_data(void)
{
  return pario_get_data();
}

// ----- send -----

static void strobe_write_func(void)
{
  // nothing more to send...
  if(send_size == 0) {
    pario_set_data(0);
    state = STROBE_FLAG_GOT_STROBE | STROBE_FLAG_ALL_SENT;
  } else {
    state = STROBE_FLAG_GOT_STROBE;

    // setup next byte
    u08 data = *send_data;
    pario_set_data(data);
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
  pario_ack_lo();
  timer_delay_1us();
  pario_ack_hi();
}

void strobe_send_begin(rom_pchar data, u16 size)
{
  // keep send range
  send_data = data;
  send_size = size;

  // setup first byte
  u08 val = *send_data;
  send_data++;
  send_size--;

  strobe_func = strobe_write_func;

  pario_busy_in();
  pario_set_data(val);
  pario_data_ddr(0xff);

  state = STROBE_FLAG_NONE;
  strobe_count = 0;
}

u08 strobe_read_flag(void)
{
  // read busy
  u08 res = pario_get_busy() ? STROBE_FLAG_IS_BUSY : STROBE_FLAG_NONE;

  __disable_irq();

  res |= state;
  state = STROBE_FLAG_NONE;

  __enable_irq();

  return res;
}

void strobe_pulse_ack(void)
{
  // pulse ack
  pario_ack_lo();
  timer_delay_1us();
  pario_ack_hi();
}

void strobe_send_end(void)
{
  pario_data_ddr(0x00);
  pario_busy_out();

  strobe_func = strobe_read_func;

  state = 0;
  strobe_count = 0;
}
