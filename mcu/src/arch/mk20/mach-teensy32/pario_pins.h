#ifndef PARIO_PINS_H
#define PARIO_PINS_H

#include <stdint.h>
#include "kinetis.h"

#define irq_off()      __disable_irq()
#define irq_on()       __enable_irq()

/* Parallel Port Setup


               Parallel Port    Teensy 3.2 Pin    Core Pin    Dir
    /STROBE        1               23               PC2        in
    DATA0          2               2                PD0
    DATA1          3               14               PD1
    DATA2          4               7                PD2
    DATA3          5               8                PD3
    DATA4          6               6                PD4
    DATA5          7               20               PD5
    DATA6          8               21               PD6
    DATA7          9               5                PD7
    /ACK          10               22               PC1        out
    BUSY          11               17               PB1        out
    POUT          12               16               PB0        in
    SELECT        13               15               PC0        in
    GND           18-22            GND
*/

/* PE/PS = pull up enable, PFE=filter enable, SRE=slew rate enable, DSE=drive strength */
#define DATA_PORT_FLAGS  PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_SRE | PORT_PCR_DSE
#define IN_PORT_FLAGS    PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE
#define OUT_PORT_FLAGS   PORT_PCR_SRE | PORT_PCR_DSE

#define DATA_MASK        0xff
#define STROBE_MASK      (1<<2)
#define ACK_MASK         (1<<1)
#define BUSY_MASK        (1<<1)
#define POUT_MASK        (1<<0)
#define SELECT_MASK      (1<<0)

#ifndef force_inline
#define force_inline     __attribute__((always_inline)) inline
#endif

static inline void pario_init(void)
{
  // ----- port mux -----
  // -- Port B:
  // POUT (IN)
  PORTB_PCR0 = PORT_PCR_MUX(1) | IN_PORT_FLAGS;
  // busy (OUT)
  PORTB_PCR1 = PORT_PCR_MUX(1) | OUT_PORT_FLAGS;

  // -- Port C:
  // SELECT (IN)
  PORTC_PCR0 = PORT_PCR_MUX(1) | IN_PORT_FLAGS;
  // ack (OUT)
  PORTC_PCR1 = PORT_PCR_MUX(1) | OUT_PORT_FLAGS;
  // strobe (in)
  PORTC_PCR2 = PORT_PCR_MUX(1) | IN_PORT_FLAGS;

  // -- Port D: data port
  PORTD_GPCLR = (DATA_MASK << 16) | PORT_PCR_MUX(1) | DATA_PORT_FLAGS;

  // ----- DDR -----
  // Port B: BUSY, POUT
  uint32_t old = GPIOB_PDDR & ~(BUSY_MASK | POUT_MASK);
  GPIOB_PDDR = old | BUSY_MASK;

  // Port C: SELECT, ACK, STROBE
  old = GPIOC_PDDR & ~(SELECT_MASK | ACK_MASK | STROBE_MASK);
  GPIOC_PDDR = old | ACK_MASK;

  // Port D: DATA
  GPIOD_PDDR &= ~0xff; // INPUT

  // ----- DATA -----
  // Port B:
  GPIOB_PSOR = BUSY_MASK | POUT_MASK;
  // Port C:
  GPIOC_PSOR = ACK_MASK | SELECT_MASK | STROBE_MASK;
  // Port D:
  GPIOD_PSOR = DATA_MASK;
}

static inline void pario_busy_out(void)
{
  PORTB_PCR1 = PORT_PCR_MUX(1) | OUT_PORT_FLAGS;

  uint32_t old = GPIOB_PDDR & ~BUSY_MASK;
  GPIOB_PDDR = old | BUSY_MASK;
}

static inline void pario_busy_in(void)
{
  PORTB_PCR1 = PORT_PCR_MUX(1) | IN_PORT_FLAGS;

  uint32_t old = GPIOB_PDDR & ~BUSY_MASK;
  GPIOB_PDDR = old;
}

static force_inline void pario_data_ddr(uint8_t ddr)
{
  GPIOD_PDDR = (GPIOD_PDDR & ~(DATA_MASK)) | ddr;
}

static force_inline uint8_t pario_get_data(void)
{
  return (uint8_t)GPIOD_PDIR;
}

static force_inline void pario_set_data(uint8_t data)
{
  GPIOD_PDOR = (GPIOD_PDOR & ~(DATA_MASK)) | data;
}

// input lines

static force_inline int pario_get_pout(void)
{
  return (GPIOB_PDIR & POUT_MASK) == POUT_MASK;
}

static force_inline int pario_get_select(void)
{
  return (GPIOC_PDIR & SELECT_MASK) == SELECT_MASK;
}

static force_inline int pario_get_strobe(void)
{
  return (GPIOC_PDIR & STROBE_MASK) == STROBE_MASK;
}

/* knok upload only */
static force_inline int pario_get_busy(void)
{
  return (GPIOB_PDIR & BUSY_MASK) == BUSY_MASK;
}

// output lines

static force_inline void pario_busy_hi(void)
{
  GPIOB_PSOR = BUSY_MASK;
}

static force_inline void pario_busy_lo(void)
{
  GPIOB_PCOR = BUSY_MASK;
}

static force_inline void pario_ack_hi(void)
{
  GPIOC_PSOR = ACK_MASK;
}

static force_inline void pario_ack_lo(void)
{
  GPIOC_PCOR = ACK_MASK;
}

#endif
