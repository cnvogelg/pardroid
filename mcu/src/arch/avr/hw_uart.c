/*
 * hw_uart.c - serial hw routines
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

#include <avr/io.h>
#include <avr/interrupt.h>

#define BAUD CONFIG_BAUD_RATE
#include <util/setbaud.h>

#include "hw_uart.h"

#ifdef UBRR0H

// for atmeg644
#define UBRRH  UBRR0H
#define UBRRL  UBRR0L
#define UCSRA  UCSR0A
#define UCSRB  UCSR0B
#define UCSRC  UCSR0C
#define UDRE   UDRE0
#define UDR    UDR0

#define RXC    RXC0
#define TXC    TXC0
#define DOR    DOR0
#define PE     UPE0

#define U2X    U2X0
#define RXEN   RXEN0
#define TXEN   TXEN0

#else
#ifdef UBRR1H

// for atmega32u4
#define UBRRH  UBRR1H
#define UBRRL  UBRR1L
#define UCSRA  UCSR1A
#define UCSRB  UCSR1B
#define UCSRC  UCSR1C
#define UDRE   UDRE1
#define UDR    UDR1

#define RXC    RXC1
#define TXC    TXC1
#define DOR    DOR1
#define PE     UPE1

#define U2X    U2X1
#define RXEN   RXEN1
#define TXEN   TXEN1

#endif
#endif

void hw_uart_init(void)
{
  // disable
  UCSRB = 0;

  UBRRH = UBRRH_VALUE;
  UBRRL = UBRRL_VALUE;
#if USE_2X
  UCSRA = (1 << U2X);
#else
  UCSRA = 0;
#endif

  UCSRB = (1 << TXEN);
  UCSRC = 0x86;
}

void hw_uart_send(u08 data)
{
  // wait for transmitter to become ready
  while(!( UCSRA & (1<<UDRE)));
  // send byte
  UDR = data;
}
