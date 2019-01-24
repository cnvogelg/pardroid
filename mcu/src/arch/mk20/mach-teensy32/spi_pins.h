#ifndef SPI_PINS_H
#define SPI_PINS_H

#include <stdint.h>
#include "kinetis.h"
#include "arch.h"

/* SPI Port Setup

              Teensy 3.2 Pin     Core Pin
   MOSI             11             PC6
   MISO             12             PC7
   SCK              13             PC5    (same as LED!)
   CS0              10             PC4
   CS1               4             PA13
*/

#define SPI_CS0_MASK    (1<<4)
#define SPI_CS1_MASK    (1<<13)

INLINE void spi_pins_init(void)
{
  // 11 MOSI
  PORTC_PCR6 = PORT_PCR_MUX(2) | PORT_PCR_DSE;
  // 12 MISO
  PORTC_PCR7 = PORT_PCR_MUX(2);
  // 13 SCK
  PORTC_PCR5 = PORT_PCR_MUX(2) | PORT_PCR_DSE;
  // 10 CS0 (regular GPIO!)
  PORTC_PCR4 = PORT_PCR_MUX(1) | PORT_PCR_DSE;
  // 4 CS1 (regular GPIO!)
  PORTA_PCR13 = PORT_PCR_MUX(1) | PORT_PCR_DSE;

  // CS0 DDR
  GPIOC_PDDR |= SPI_CS0_MASK;
  // CS1 DDR
  GPIOA_PDDR |= SPI_CS1_MASK;

  // set CS0 HI
  GPIOC_PSOR = SPI_CS0_MASK;
  // set CS1 HI
  GPIOA_PSOR = SPI_CS1_MASK;
}

FORCE_INLINE void spi_pins_cs0_hi(void)
{
  GPIOC_PSOR = SPI_CS0_MASK;
}

FORCE_INLINE void spi_pins_cs0_lo(void)
{
  GPIOC_PCOR = SPI_CS0_MASK;
}

FORCE_INLINE void spi_pins_cs1_hi(void)
{
  GPIOA_PSOR = SPI_CS1_MASK;
}

FORCE_INLINE void spi_pins_cs1_lo(void)
{
  GPIOA_PCOR = SPI_CS1_MASK;
}

#endif
