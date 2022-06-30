#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "hw_uart.h"
#include "hw_system.h"
#include "hw_timer.h"

#include "uartutil.h"
#include "rominfo.h"
#include "fwid.h"
#include "fw_info.h"

#include "pamela.h"
#include "paloma.h"
#include "test/paloma.h"
#include "paloma_api.h"
#include "paloma/types.h"

// define my app id
FW_INFO(FWID_TEST_PALOMA, VERSION_TAG)

// ----- paloma API -----

void paloma_api_param_all_reset(void)
{
  uart_send_pstring(PSTR("all_reset"));
  uart_send_crlf();
}

void paloma_api_param_all_load(void)
{
  uart_send_pstring(PSTR("all_load"));
  uart_send_crlf();
}

void paloma_api_param_all_save(void)
{
  uart_send_pstring(PSTR("all_save"));
  uart_send_crlf();
}

u08 paloma_api_param_get_total_slots(void)
{
  return NUM_TEST_SLOTS;
}

u08 paloma_api_param_get_id(u08 slot)
{
  return 0;
}

u08 paloma_api_param_get_type(u08 slot)
{
  return PALOMA_TYPE_UBYTE;
}

void paloma_api_param_get_min_max_bytes(u08 slot, u08 *min, u08 *max)
{
  *min = 2;
  *max = 8;
}

void paloma_api_param_get_info(u08 slot, paloma_param_info_t *info)
{

}

u08 paloma_api_param_get_ubyte(u08 slot, u08 def)
{
  return 42;
}

void paloma_api_param_set_ubyte(u08 slot, u08 val)
{

}

u16 paloma_api_param_get_uword(u08 slot, u08 def)
{
  return 0xcafe;
}

void paloma_api_param_set_uword(u08 slot, u16 val)
{

}

u32 paloma_api_param_get_ulong(u08 slot, u08 def)
{
  return 0xdeadbeef;
}

void paloma_api_param_set_ulong(u08 slot, u32 val)
{

}

u08 paloma_api_param_get_buffer(u08 slot, u08 *data)
{
  data[0] = 0x11;
  data[1] = 0x22;
  data[2] = 0x33;
  data[3] = 0x44;
  return 4;
}

void paloma_api_param_set_buffer(u08 slot, u08 *data, u08 size)
{

}

// ----- main -----

int main(void)
{
  hw_system_init();
  hw_uart_init();

  uart_send_pstring(PSTR("parbox: test-paloma!"));
  uart_send_crlf();

  rom_info();

  pamela_init();
  paloma_init();

  while(1) {
    hw_system_wdt_reset();
    pamela_work();
#ifdef VERBOSE
    uart_send('.');
#endif
 }

  return 0;
}
