#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "uart.h"
#include "uartutil.h"
#include "pablo.h"
#include "machtag.h"
#include "fwid.h"

void rom_info(void)
{
  // show pablo infos
  u16 crc = pablo_check_rom_crc();
  u16 mt  = pablo_get_mach_tag();
  u16 ver = pablo_get_rom_version();
  u16 id  = pablo_get_rom_fw_id();
  uart_send_pstring(PSTR("ROM: crc="));
  uart_send_hex_word(crc);
  uart_send_pstring(PSTR(" machtag="));
  uart_send_hex_word(mt);
  uart_send_pstring(PSTR(" firmware: id="));
  uart_send_hex_word(id);
  uart_send_pstring(PSTR(" ver="));
  uart_send_hex_word(ver);
  uart_send_crlf();

  // decode machtag
  rom_pchar arch,mcu,mach;
  u08 extra;
  machtag_decode(mt, &arch, &mcu, &mach, &extra);
  uart_send_pstring(arch);
  uart_send('-');
  uart_send_pstring(mcu);
  uart_send('-');
  uart_send_pstring(mach);
  uart_send('-');
  uart_send_hex_byte(extra);
  uart_send_crlf();

  // decode fwid
  rom_pchar id_str;
  fwid_decode(id, &id_str);
  uart_send_pstring(id_str);
  uart_send_crlf();
}
