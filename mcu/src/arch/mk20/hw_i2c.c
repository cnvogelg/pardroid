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

#include "kinetis.h"

#include "autoconf.h"
#include "types.h"

#include "hw_i2c.h"

#if F_BUS != 36000000
#error F_BUS
#endif

/* I2C speed 

  F = 0x28 -> 113 kHz
  F = 0x19 -> 375 kHz
  F = 0x0a -> 1 MHz

*/

/* I2C Pins 

// 18	B3			SDA
// 19	B2			SCL

*/

void hw_i2c_init(void)
{
  // configure pins
  PORTB_PCR3 = PORT_PCR_MUX(2) | PORT_PCR_ODE|PORT_PCR_SRE|PORT_PCR_DSE;
  PORTB_PCR2 = PORT_PCR_MUX(2) | PORT_PCR_ODE|PORT_PCR_SRE|PORT_PCR_DSE;

  // enable I2C module clock
  SIM_SCGC4 |= SIM_SCGC4_I2C0;

  // set frequency
  I2C0_F = 0x19;

  // glitch filter
  I2C0_FLT = 3;

  // my addr
  I2C0_A1 = 0x23;

  // high drive select
  I2C0_C2 = I2C_C2_HDRS;

  // enable module
  I2C0_C1 = I2C_C1_IICEN;
}

u08 hw_i2c_start(u08 addr, u08 write)
{
  // make sure bus is idle
  while(I2C0_S & I2C_S_BUSY);

  // enable with master and Tx
  I2C0_C1  = I2C_C1_IICEN | I2C_C1_MST | I2C_C1_TX;

  // write address
  u08 data = addr << 1;
  if(!write) data |= 1;
  I2C0_D = data;

  // wait for busy
  while(!(I2C0_S & I2C_S_BUSY));

  // wait for completion
  while(!(I2C0_S & I2C_S_IICIF));
  // confirm IIR
  I2C0_S |= I2C_S_IICIF;

  // setup read mode
  if(!write) {
    // remove tx flag
    I2C0_C1  = I2C_C1_IICEN | I2C_C1_MST;

    // trigger read
    (void) I2C0_D;
  }

  return 0;
}

void hw_i2c_stop(void)
{
  // remove MST to trigger stop condition
  I2C0_C1 = I2C_C1_IICEN;
}

u08 hw_i2c_write_byte(u08 data)
{
  // write data
  I2C0_D = data;

  // wait for completion
  while(!(I2C0_S & I2C_S_IICIF));
  // confirm IIR
  I2C0_S |= I2C_S_IICIF;

  return 0;
}

u08 hw_i2c_read_byte(u08 ack, u08 *data)
{
  // wait for completion
  while(!(I2C0_S & I2C_S_IICIF));
  // confirm IIR
  I2C0_S |= I2C_S_IICIF;
 
  // read data
  *data = I2C0_D;
	return 0;
}

u08 hw_i2c_write(u08 addr, const u08 *data, u16 len)
{
	u08 res;

	res = hw_i2c_start(addr, 1);
	if(res)
		return res;

	for(u16 i=0;i<len;i++) {
		res = hw_i2c_write_byte(data[i]);
    if(res != 0) {
      break;
    }
	}

	hw_i2c_stop();
	return res;
}

u08 hw_i2c_read(u08 addr, u08 *data, u16 len)
{
	u08 res;

	res = hw_i2c_start(addr, 0);
	if(res)
		return res;
	
	for(u16 i=len;i>=0;i--) {
		res = hw_i2c_read_byte(i!=0, &data[i]);
    if(res != 0) {
      break;
    }
	}

	hw_i2c_stop();
	return res;
}
