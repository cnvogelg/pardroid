#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "uart.h"
#include "uartutil.h"
#include "proto.h"
#include "debug.h"

int main(void)
{
  uart_init();
  uart_send_pstring(PSTR("parbox-test!\n"));

  DC('+');
  proto_init();
  DC('-');

  while(1) {
    proto_handle();
  }
  return 0;
}
