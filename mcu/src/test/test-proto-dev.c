#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "debug.h"

#include "hw_uart.h"
#include "hw_system.h"
#include "hw_led.h"
#include "hw_timer.h"

#include "uartutil.h"
#include "rominfo.h"

#include "proto_atom.h"
#include "proto_dev.h"

#include "fwid.h"
#include "fw_info.h"

FW_INFO(FWID_TEST_PROTO_DEV, VERSION_TAG)

// ----- main -----

int main(void)
{
  hw_system_init();
  hw_led_init();
  hw_uart_init();

  uart_send_pstring(PSTR("\r\n\r\n-----\r\nparbox: test-proto-dev!"));
  uart_send_crlf();

  rom_info();

  proto_dev_init();

  while(1) {
      u08 cmd = proto_dev_get_cmd();
      if(cmd != PROTO_NO_CMD) {
        // invalid command
        uart_send('#');
        uart_send_hex_byte(cmd);
        uart_send_crlf();
      }
  }

  return 0;
}
