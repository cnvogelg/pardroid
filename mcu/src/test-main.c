#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "uart.h"
#include "uartutil.h"
#include "proto.h"
#include "debug.h"
#include "mach.h"

#include <util/delay.h>

int main(void)
{
  mach_init_hw();

  uart_init();
  uart_send_pstring(PSTR("parbox-test!\n"));
  _delay_ms(300);

  DC('+');
  proto_init();
  DC('-');

  while(1) {
    proto_handle();
#if 0
    _delay_ms(500);
    DC('.');
#endif
  }

  return 0;
}
