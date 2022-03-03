#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "uartutil.h"
#include "rominfo.h"
#include "fw_info.h"

const char rom_info_str[] ROM_ATTR = ROMINFO_STRING;

void rom_info(void)
{
  uart_send_pstring(rom_info_str);
  uart_send_crlf();

  // firmware info
  uart_send_pstring(PSTR("fwid:"));
  uart_send_hex_word(FW_GET_ID());
  uart_send_pstring(PSTR(",mach:"));
  uart_send_hex_word(FW_GET_MACHTAG());
  uart_send_pstring(PSTR(",vers:"));
  uart_send_hex_word(FW_GET_VERSION());
  uart_send_crlf();
}
