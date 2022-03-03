/*
 * hw_i2c.c - I2C setup
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

#include "hardware/i2c.h"
#include "hardware/gpio.h"

#include "autoconf.h"
#include "types.h"

#include "hw_i2c.h"
#include "hw_i2c_pins.h"

void hw_i2c_init(void)
{
  i2c_init(i2c0, 100 * 1000);

  gpio_set_function(HW_I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_set_function(HW_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(HW_I2C_SCL_PIN);
  gpio_pull_up(HW_I2C_SDA_PIN);
}

u08 hw_i2c_write(u08 addr, const u08 *data, u16 len)
{
  i2c_write_blocking(i2c0, addr, data, len, false);
}

u08 hw_i2c_read(u08 addr, u08 *data, u16 len)
{
  i2c_read_blocking(i2c0, addr, data, len, false);
}

