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

#include <avr/io.h>
#include <util/twi.h>

#include "autoconf.h"
#include "types.h"

#include "hw_i2c.h"

// i2c freq
#define F_SCL 100000UL
#define PRE_SCALE 1
#define TWI_CNT ((((F_CPU / F_SCL) / PRE_SCALE) - 16 ) / 2)

void hw_i2c_init(void)
{
  TWBR = (u08)TWI_CNT;
}

static u08 i2c_start(u08 addr, u08 write)
{
	TWCR = 0;
	// send start condition
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while( !(TWCR & (1<<TWINT)) );
	// check if start condition mas met
	if((TWSR & 0xF8) != TW_START){ return 1; }
	
	// set slave addr and transmit it
	u08 val = addr << 1;
	if(write)
		val |= TW_WRITE;
	TWDR = val;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while( !(TWCR & (1<<TWINT)) );	
	// check if the device has acknowledged the READ / WRITE mode
	uint8_t twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) {
		return 2;
	}
	return 0;
}

static void i2c_stop(void)
{
	// transmit STOP condition
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
}

static u08 i2c_write_byte(u08 data)
{
	// load data and transmit
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while( !(TWCR & (1<<TWINT)) );
	// check status	
	if( (TWSR & 0xF8) != TW_MT_DATA_ACK ) { 
		return 1; 
	}
	return 0;
}

static u08 i2c_read_byte(u08 ack, u08 *data)
{
	// start read
	u08 val = (1<<TWINT) | (1<<TWEN); 
	if(ack) {
		val |=  (1<<TWEA);
	}
	TWCR = val;
	while( !(TWCR & (1<<TWINT)) );
	*data = TWDR;
	return 0;
}

u08 hw_i2c_write(u08 addr, const u08 *data, u16 len)
{
	u08 res;

	res = i2c_start(addr, 1);
	if(res)
		return res;

	for(u16 i=0;i<len;i++) {
		res = i2c_write_byte(data[i]);
		if(res)
			break;
	}

	i2c_stop();
	return res;
}

u08 hw_i2c_write_rom(u08 addr, rom_pchar data, u16 len)
{
  u08 res;

  res = i2c_start(addr, 1);
  if(res)
    return res;

  for(u16 i=0;i<len;i++) {
    u08 ch = read_rom_char(data);
    data++;
    res = i2c_write_byte(ch);
    if(res)
      break;
  }

  i2c_stop();
  return res;
}

u08 hw_i2c_read(u08 addr, u08 *data, u16 len)
{
	u08 res;

	res = i2c_start(addr, 0);
	if(res)
		return res;
	
	for(u16 i=len;i>=0;i--) {
		res = i2c_read_byte(i!=0,&data[i]);
		if(res)
			break;
	}

	i2c_stop();
	return res;
}
