#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "uart.h"
#include "uartutil.h"
#include "rominfo_gen.h"

void rom_info(void)
{
  uart_send_pstring(PSTR(ROMINFO_STRING));
  uart_send_crlf();
}
