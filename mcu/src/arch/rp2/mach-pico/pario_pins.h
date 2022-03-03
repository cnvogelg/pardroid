#ifndef PARIO_PINS_H
#define PARIO_PINS_H

#include "pico/stdlib.h"
#include "arch.h"

#define irq_off()
#define irq_on()

/* Parallel Port Setup

               Parallel Port    Pico GPIO      Dir
    /STROBE        1               10          in
    DATA0          2               2
    DATA1          3               3
    DATA2          4               4
    DATA3          5               5
    DATA4          6               6
    DATA5          7               7
    DATA6          8               8
    DATA7          9               9
    /ACK          10               11          out
    BUSY          11               12          out
    POUT          12               13          in
    SELECT        13               14          in
    GND           18-22            GND
*/

#define PAR_STROBE_PIN    10
#define PAR_ACK_PIN       11
#define PAR_BUSY_PIN      12
#define PAR_POUT_PIN      13
#define PAR_SELECT_PIN    14

#define PAR_DATA_MASK     (0xff << 2)

INLINE void pario_init(void)
{
  // strobe in
  gpio_init(PAR_STROBE_PIN);
  gpio_set_dir(PAR_STROBE_PIN, GPIO_IN);

  // ACK out hi
  gpio_init(PAR_ACK_PIN);
  gpio_put(PAR_ACK_PIN, true);
  gpio_set_dir(PAR_ACK_PIN, GPIO_OUT);

  // BUSY out lo
  gpio_init(PAR_BUSY_PIN);
  gpio_put(PAR_BUSY_PIN, false);
  gpio_set_dir(PAR_BUSY_PIN, GPIO_OUT);

  // POUT in
  gpio_init(PAR_POUT_PIN);
  gpio_set_dir(PAR_POUT_PIN, GPIO_IN);

  // SELECT in
  gpio_init(PAR_SELECT_PIN);
  gpio_set_dir(PAR_SELECT_PIN, GPIO_IN);

  // data (in)
  gpio_init_mask(PAR_DATA_MASK);
  gpio_set_dir_in_masked(PAR_DATA_MASK);
}

INLINE void pario_busy_out(void)
{
  gpio_set_dir(PAR_BUSY_PIN, GPIO_OUT);
}

INLINE void pario_busy_in(void)
{
  gpio_set_dir(PAR_BUSY_PIN, GPIO_IN);
}

FORCE_INLINE void pario_data_ddr_out(void)
{
  gpio_set_dir_out_masked(PAR_DATA_MASK);
}

FORCE_INLINE void pario_data_ddr_in(void)
{
  gpio_set_dir_in_masked(PAR_DATA_MASK);
}

FORCE_INLINE uint8_t pario_get_data(void)
{
  uint32_t val = gpio_get_all();
  return (uint8_t)((val >> 2) & 0xff);
}

FORCE_INLINE void pario_set_data(uint8_t data)
{
  gpio_put_masked(PAR_DATA_MASK, data << 2);
}

// input lines

FORCE_INLINE int pario_get_pout(void)
{
  return gpio_get(PAR_POUT_PIN);
}

FORCE_INLINE int pario_get_select(void)
{
  return gpio_get(PAR_SELECT_PIN);
}

FORCE_INLINE int pario_get_strobe(void)
{
  return gpio_get(PAR_STROBE_PIN);
}

/* knok upload only */
FORCE_INLINE int pario_get_busy(void)
{
  return gpio_get(PAR_BUSY_PIN);
}

// output lines

FORCE_INLINE void pario_busy_hi(void)
{
  gpio_put(PAR_BUSY_PIN, true);
}

FORCE_INLINE void pario_busy_lo(void)
{
  gpio_put(PAR_BUSY_PIN, false);
}

FORCE_INLINE void pario_pout_hi(void)
{
  gpio_put(PAR_POUT_PIN, true);
}

FORCE_INLINE void pario_pout_lo(void)
{
  gpio_put(PAR_POUT_PIN, false);
}

FORCE_INLINE void pario_ack_hi(void)
{
  gpio_put(PAR_ACK_PIN, true);
}

FORCE_INLINE void pario_ack_lo(void)
{
  gpio_put(PAR_ACK_PIN, false);
}

#endif
