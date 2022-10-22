/*
 * uartutil.c - serial utility routines
 *
 * Written by
 *  Christian Vogelgsang <chris@vogelgsang.org>
 *
 * This file is part of plipbox.
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

#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "hw_uart.h"
#include "uartutil.h"

void uart_send_pstring(rom_pchar data)
{
  while(1) {
    u08 c = read_rom_char(data);
    if(c == 0) {
      break;
    }
    hw_uart_send(c);
    data++;
  }
}

void uart_send_string(const char *str)
{
  while(*str) {
    hw_uart_send((u08)*str);
    str++;
  }
}

void uart_send_data(u08 *data,u08 len)
{
  for(u08 i=0;i<len;i++) {
    hw_uart_send(data[i]);
  }
}

void uart_send_crlf(void)
{
  hw_uart_send(13);
  hw_uart_send(10);
}

void uart_send_spc(void)
{
  hw_uart_send((u08)' ');
}

static u08 nybble_to_hex(u08 in)
{
  if(in<10)
    return '0' + in;
  else
    return 'A' + in - 10;
}

void uart_send_hex_byte(u08 in)
{
  hw_uart_send(nybble_to_hex(in >> 4));
  hw_uart_send(nybble_to_hex(in & 0xf));
}

void uart_send_hex_word(u16 in)
{
  uart_send_hex_byte((u08)(in>>8));
  uart_send_hex_byte((u08)(in&0xff));
}

void uart_send_hex_long(u32 in)
{
  uart_send_hex_word((u16)(in>>16));
  uart_send_hex_word((u16)(in&0xffff));
}

void uart_send_hex_dump(u32 offset, const u08 *data, u16 size)
{
  while(size > 0) {
    u16 step;
    if(size > 16) {
      step = 16;
    } else {
      step = size;
    }
    uart_send_hex_line(offset, data, step);
    data += step;
    size -= step;
  }
}

void uart_send_hex_line(u32 offset, const u08 *data, u08 size)
{
  if(size > 16) {
    size = 16;
  }
  u08 remainder = 16 - size;

  // offset
  uart_send_hex_long(offset);
  uart_send_pstring(PSTR(": "));

  // hex bytes
  for(u08 i=0;i<size;i++) {
    uart_send_hex_byte(data[i]);
    hw_uart_send(' ');
  }
  for(u08 i=0;i<remainder;i++) {
    uart_send_pstring(PSTR("   "));
  }

  // ascii
  uart_send_pstring(PSTR("    "));
  for(u08 i=0;i<size;i++) {
    u08 val = data[i];
    if((val < 32) || (val > 126)) val = '.';
    hw_uart_send(val);
  }

  uart_send_crlf();
}
