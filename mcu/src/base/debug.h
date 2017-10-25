#ifndef DEBUG_H
#define DEBUG_H

#include "autoconf.h"

#ifdef CONFIG_DEBUG
// enable debug output macros
#include "uart.h"
#include "uartutil.h"

// debug char
#define DC(x)  uart_send(x)
// debug string
#define DS(x)  uart_send_pstring(PSTR(x))
// debug byte
#define DB(x)  uart_send_hex_byte(x)
// debug word
#define DW(x)  uart_send_hex_word(x)
// debug long
#define DL(x)  uart_send_hex_long(x)
// debug newline
#define DNL    uart_send_crlf()
// debug space
#define DSPC   uart_send(' ')

#else
// debug output is disabled

#define DC(x)
#define DS(x)
#define DB(x)
#define DW(x)
#define DL(x)
#define DNL
#define DSPC

#endif

#endif
