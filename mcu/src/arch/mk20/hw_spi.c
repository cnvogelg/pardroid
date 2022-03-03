/*
 * spi.c - SPI setup
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of parbox.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "hw_spi.h"

// missing SPI defines
#define SPI_SR_RXCTR 0XF0
#define SPI_PUSHR_CTAS(n) (((n) & 7) << 28)

// PBR   -> prescale baud rate FBUS/x
// PBR=0 -> x=2
// PBR=1 -> x=3
// PBR=2 -> x=5
// PBR=3 -> x=7

// DBR=1  double rate (1+x), DBR=0  normal rate

// BR   -> baudrate scalar
// 0       2
// 1       4
// 2       8
// 3       16
// 4       32
// 5       64
// ...
// 15      65536

// F_SPI = (F_BUS / PBR) * ((1+DBR) / BR)

// F_BUS=36 MHz
#define SPI_CLOCK_24MHz   (SPI_CTAR_PBR(0) | SPI_CTAR_BR(0) | SPI_CTAR_DBR) //(36 / 2) * ((1+1)/2) = 18
#define SPI_CLOCK_16MHz   (SPI_CTAR_PBR(1) | SPI_CTAR_BR(0) | SPI_CTAR_DBR) //(36 / 3) * ((1+1)/2) = 12
#define SPI_CLOCK_12MHz   (SPI_CTAR_PBR(1) | SPI_CTAR_BR(0) | SPI_CTAR_DBR) //(36 / 3) * ((1+1)/2) = 12
#define SPI_CLOCK_8MHz    (SPI_CTAR_PBR(2) | SPI_CTAR_BR(0) | SPI_CTAR_DBR) //(36 / 5) * ((1+1)/2) = 7.2
#define SPI_CLOCK_6MHz    (SPI_CTAR_PBR(0) | SPI_CTAR_BR(2) | SPI_CTAR_DBR) //(36 / 2) * ((1+1)/6)
#define SPI_CLOCK_4MHz    (SPI_CTAR_PBR(1) | SPI_CTAR_BR(2) | SPI_CTAR_DBR) //(36 / 3) * ((1+1)/6)
#define SPI_CLOCK_DIV_96  (SPI_CTAR_PBR(1) | SPI_CTAR_BR(5))

// slow
#define SPI_CLOCK_SLOW    SPI_CLOCK_DIV_96
// fast
#define SPI_CLOCK_FAST    SPI_CLOCK_12MHz

#if F_BUS != 36000000
#error F_BUS
#endif

void hw_spi_init(void)
{
  // enable SPI module clock
  SIM_SCGC6 |= SIM_SCGC6_SPI0;

  // setup pin mux
  spi_pins_init();

  uint32_t ctar0 = SPI_CLOCK_FAST | SPI_CTAR_FMSZ(7);
  ctar0 |= (ctar0 & 0x0f) << 12;

  // setup attributes
  SPI0_MCR = SPI_MCR_MDIS | SPI_MCR_HALT | SPI_MCR_PCSIS(0x1F);
  // attributes 0: slow, 8 bit
  SPI0_CTAR0 = ctar0;
  // start SPI engine, clear FIFOs
  SPI0_MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F) | SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF;
}

void hw_spi_set_speed(u08 s)
{
  uint32_t speed;
  if(s == SPI_SPEED_MAX) {
    speed = SPI_CLOCK_FAST;
  } else {
    speed = SPI_CLOCK_SLOW;
  }

  uint32_t ctar0 = speed | SPI_CTAR_FMSZ(7);
  ctar0 |= (ctar0 & 0x0f) << 12;

  // setup attributes
  SPI0_MCR = SPI_MCR_MDIS | SPI_MCR_HALT | SPI_MCR_PCSIS(0x1F);
  // attributes 0: slow, 8 bit
  SPI0_CTAR0 = ctar0;
  // start SPI engine, clear FIFOs
  SPI0_MCR = SPI_MCR_MSTR | SPI_MCR_PCSIS(0x1F) | SPI_MCR_CLR_TXF | SPI_MCR_CLR_RXF;
}

u08 hw_spi_xfer(u08 data)
{
  SPI0_MCR |= SPI_MCR_CLR_RXF;
  SPI0_SR = SPI_SR_TCF;
  SPI0_PUSHR = data;
  while (!(SPI0_SR & SPI_SR_TCF)) {}
  return SPI0_POPR;
}
