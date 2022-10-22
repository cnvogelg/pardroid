/*
 * uartutil.h - serial utility routines
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

#ifndef UARTUTIL_H
#define UARTUTIL_H

#include "hw_uart.h"
#include "arch.h"

INLINE void uart_send(u08 data) { hw_uart_send(data); }

// send a c string from PROGMEM
void uart_send_pstring(rom_pchar data);
// send a c string
void uart_send_string(const char *data);
// send data
void uart_send_data(u08 *data,u08 size);
// send a CR+LF
void uart_send_crlf(void);
// send a Space
void uart_send_spc(void);

// send a hex byte
void uart_send_hex_byte(u08 data);
// send a hex word
void uart_send_hex_word(u16 data);
// send a hex word
void uart_send_hex_long(u32 data);

// send a hex dump
void uart_send_hex_dump(u32 offset, const u08 *data, u16 size);
void uart_send_hex_line(u32 offset, const u08 *data, u08 size);


#if CONFIG_PTR_BITS == 16
#define uart_send_hex_ptr(x)  uart_send_hex_word((u16)x)
#elif CONFIG_PTR_BITS == 32
#define uart_send_hex_ptr(x)  uart_send_hex_long((u32)x)
#else
#error invalid CONFIG_PTR_BITS
#endif

#endif

