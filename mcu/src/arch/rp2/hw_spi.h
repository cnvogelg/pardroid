/*
 * hw_spi.h - SPI setup
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

#ifndef HW_SPI_H
#define HW_SPI_H

#include "autoconf.h"

#include "types.h"
#include "hw_spi_pins.h"
#include "hw_spi_common.h"

extern void hw_spi_init(void);
extern void hw_spi_set_speed(u08 speed);

extern u08 hw_spi_xfer(u08 data);
INLINE void hw_spi_out(u08 data) { hw_spi_xfer(data); }
INLINE u08 hw_spi_in(void) { return hw_spi_xfer(0xff); }

#define hw_spi_enable_cs0()  hw_spi_pins_cs0_lo()
#define hw_spi_disable_cs0() hw_spi_pins_cs0_hi()

#define hw_spi_enable_cs1()  hw_spi_pins_cs1_lo()
#define hw_spi_disable_cs1() hw_spi_pins_cs1_hi()

#endif // SPI_H
