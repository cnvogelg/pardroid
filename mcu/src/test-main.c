#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "uart.h"
#include "uartutil.h"
#include "proto.h"

int main(void)
{
  uart_init();
  uart_send_pstring(PSTR("parbox:test!\n"));
  proto_init();
  while(1) {
    proto_handle();
  }
  return 0;
}
