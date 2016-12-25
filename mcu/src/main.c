#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "uart.h"
#include "uartutil.h"

int main(void)
{
  uart_init();
  uart_send_pstring(PSTR("hello, world!\n"));
  while(1) {
  }
  return 0;
}
