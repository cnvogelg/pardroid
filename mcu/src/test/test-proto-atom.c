#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "debug.h"

#include "uart.h"
#include "uartutil.h"
#include "rominfo.h"
#include "system.h"
#include "led.h"
#include "timer.h"

#include "proto_atom.h"
#include "proto_atom_test_shared.h"

#include "fwid.h"
#include "fw_info.h"
#include "spi.h"

FW_INFO(FWID_TEST_PROTO_ATOM, VERSION_TAG)

u08 buf[TEST_BUF_SIZE];

static void buf_init(void)
{
  for(u16 i=0;i<TEST_BUF_SIZE;i++) {
    buf[i] = (u08)(i & 0xff);
  }
}

static void handle_cmd(u08 cmd)
{
  u16 wval;
  u32 lval;

  switch(cmd) {
    case TEST_ACTION:
      proto_atom_action();
#ifdef FLAVOR_DEBUG
      uart_send_pstring(PSTR("action!"));
      uart_send_crlf();
#endif
      break;

    case TEST_READ_WORD:
      proto_atom_read_word(TEST_WORD);
#ifdef FLAVOR_DEBUG
      uart_send_pstring(PSTR("read word"));
      uart_send_crlf();
#endif
      break;

    case TEST_WRITE_WORD:
      wval = proto_atom_write_word();
#ifdef FLAVOR_DEBUG
      uart_send_pstring(PSTR("write word: "));
      uart_send_hex_word(wval);
      uart_send_crlf();
#endif
      break;

    case TEST_READ_LONG:
      proto_atom_read_long(TEST_LONG);
#ifdef FLAVOR_DEBUG
      uart_send_pstring(PSTR("read long"));
      uart_send_crlf();
#endif
      break;

    case TEST_WRITE_LONG:
      lval = proto_atom_write_long();
#ifdef FLAVOR_DEBUG
      uart_send_pstring(PSTR("write long: "));
      uart_send_hex_long(lval);
      uart_send_crlf();
#endif
      break;

    case TEST_READ_BLOCK:
      proto_atom_read_block(buf, TEST_BUF_SIZE);
#ifdef FLAVOR_DEBUG
      uart_send_pstring(PSTR("read block"));
      uart_send_crlf();
#endif
      break;

    case TEST_WRITE_BLOCK:
      proto_atom_write_block(buf, TEST_BUF_SIZE);
#ifdef FLAVOR_DEBUG
      uart_send_pstring(PSTR("write block"));
      uart_send_crlf();
#endif
      break;

    case TEST_READ_BLOCK_SPI:
      proto_atom_read_block(NULL, TEST_BUF_SIZE);
#ifdef FLAVOR_DEBUG
      uart_send_pstring(PSTR("read block SPI"));
      uart_send_crlf();
#endif
      break;

    case TEST_WRITE_BLOCK_SPI:
      proto_atom_write_block(NULL, TEST_BUF_SIZE);
#ifdef FLAVOR_DEBUG
      uart_send_pstring(PSTR("write block SPI"));
      uart_send_crlf();
#endif
      break;

    case TEST_PULSE_IRQ:
      proto_atom_action();
      proto_atom_pulse_irq();
#ifdef FLAVOR_DEBUG
      uart_send_pstring(PSTR("pulse IRQ"));
      uart_send_crlf();
#endif
      break;

    case TEST_SET_BUSY:
      proto_atom_action();
      proto_atom_set_busy();
#ifdef FLAVOR_DEBUG
      uart_send_pstring(PSTR("set busy"));
      uart_send_crlf();
#endif
      break;

    case TEST_CLR_BUSY:
      proto_atom_action();
      proto_atom_clr_busy();
#ifdef FLAVOR_DEBUG
      uart_send_pstring(PSTR("set busy"));
      uart_send_crlf();
#endif
      break;

    default:
      uart_send_pstring(PSTR("?cmd:"));
      uart_send_hex_byte(cmd);
      uart_send_crlf();
      break;
  }
}

// ----- main -----

int main(void)
{
  system_init();
  led_init();
  spi_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-proto-atom!"));
  uart_send_crlf();

  rom_info();

  proto_atom_init();

  buf_init();

  while(1) {
      u08 cmd = proto_atom_get_cmd();
      if(cmd != PROTO_NO_CMD) {
        handle_cmd(cmd);
      }

      system_wdt_reset();
  }

  return 0;
}
