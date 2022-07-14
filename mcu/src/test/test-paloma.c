#include <string.h>

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

// test data
static u08 test_ubyte = TEST_UBYTE_VAL;
static u16 test_uword = TEST_UWORD_VAL;
static u32 test_ulong = TEST_ULONG_VAL;
static u08 test_buf[TEST_BUF_MAX];
static u08 test_buf_size = TEST_BUF_MIN;

// save slots
static u08 save_ubyte = TEST_UBYTE_VAL;
static u16 save_uword = TEST_UWORD_VAL;
static u32 save_ulong = TEST_ULONG_VAL;
static u08 save_buf[TEST_BUF_MAX];
static u08 save_buf_size = TEST_BUF_MIN;

// define my app id
FW_INFO(FWID_TEST_PALOMA, VERSION_TAG)

// ----- paloma API -----

void paloma_api_param_all_reset(void)
{
  uart_send_pstring(PSTR("all_reset"));
  uart_send_crlf();

  test_ubyte = TEST_UBYTE_VAL;
  test_uword = TEST_UWORD_VAL;
  test_ulong = TEST_ULONG_VAL;
  for(u08 i=0;i<TEST_BUF_MAX;i++) {
    test_buf[i] = 0;
  }
  test_buf_size = TEST_BUF_MIN;
}

void paloma_api_param_all_load(void)
{
  uart_send_pstring(PSTR("all_load"));
  uart_send_crlf();

  test_ubyte = save_ubyte;
  test_uword = save_uword;
  test_ulong = save_ulong;
  for(u08 i=0;i<TEST_BUF_MAX;i++) {
    test_buf[i] = save_buf[i];
  }
  test_buf_size = save_buf_size;
}

void paloma_api_param_all_save(void)
{
  uart_send_pstring(PSTR("all_save"));
  uart_send_crlf();

  save_ubyte = test_ubyte;
  save_uword = test_uword;
  save_ulong = test_ulong;
  for(u08 i=0;i<TEST_BUF_MAX;i++) {
    save_buf[i] = test_buf[i];
  }
  save_buf_size = test_buf_size;
}

u08 paloma_api_param_get_total_slots(void)
{
  uart_send_pstring(PSTR("get_total_slots: "));
  uart_send_hex_byte(NUM_TEST_SLOTS);
  uart_send_crlf();

  return NUM_TEST_SLOTS;
}

u08 paloma_api_param_get_id(u08 slot)
{
  u08 id = slot * 2;

  uart_send_pstring(PSTR("get_id: "));
  uart_send_hex_byte(slot);
  uart_send(':');
  uart_send_hex_byte(id);
  uart_send_crlf();

  return id;
}

u08 paloma_api_param_get_type(u08 slot)
{
  u08 type;
  switch(slot) {
    case TEST_SLOT_UBYTE:
      type = PALOMA_TYPE_UBYTE;
      break;
    case TEST_SLOT_UWORD:
      type = PALOMA_TYPE_UWORD;
      break;
    case TEST_SLOT_ULONG:
      type = PALOMA_TYPE_ULONG;
      break;
    case TEST_SLOT_BUF:
      type = PALOMA_TYPE_BUFFER;
      break;
  }

  uart_send_pstring(PSTR("get_type: "));
  uart_send_hex_byte(slot);
  uart_send(':');
  uart_send_hex_byte(type);
  uart_send_crlf();

  return type;
}

void paloma_api_param_get_min_max_bytes(u08 slot, u08 *min, u08 *max)
{
  switch(slot) {
    case TEST_SLOT_BUF:
      *min = TEST_BUF_MIN;
      *max = TEST_BUF_MAX;
      break;
    default:
      *min = 0;
      *max = 0;
      break;
  }

  uart_send_pstring(PSTR("get_min_max_bytes: "));
  uart_send_hex_byte(slot);
  uart_send(':');
  uart_send_hex_byte(*min);
  uart_send(',');
  uart_send_hex_byte(*max);
  uart_send_crlf();
}

void paloma_api_param_get_info(u08 slot, paloma_param_info_t *info)
{
  char *name = NULL;
  u08 type;
  info->min_bytes = 0;
  info->max_bytes = 0;
  switch(slot) {
    case TEST_SLOT_UBYTE:
      name = TEST_SLOT_UBYTE_NAME;
      type = PALOMA_TYPE_UBYTE;
      break;
    case TEST_SLOT_UWORD:
      name = TEST_SLOT_UWORD_NAME;
      type = PALOMA_TYPE_UWORD;
      break;
    case TEST_SLOT_ULONG:
      name = TEST_SLOT_ULONG_NAME;
      type = PALOMA_TYPE_ULONG;
      break;
    case TEST_SLOT_BUF:
      name = TEST_SLOT_BUF_NAME;
      type = PALOMA_TYPE_BUFFER;
      info->min_bytes = TEST_BUF_MIN;
      info->max_bytes = TEST_BUF_MAX;
      break;
  }
  strcpy(info->name, name);
  info->id = slot * 2;
  info->type = type;

  uart_send_pstring(PSTR("get_info: "));
  uart_send_hex_byte(slot);
  uart_send_crlf();
}

u08 paloma_api_param_get_ubyte(u08 slot, u08 def)
{
  u08 val = def ? TEST_UBYTE_VAL : test_ubyte;

  uart_send_pstring(PSTR("get_ubyte: "));
  uart_send_hex_byte(slot);
  uart_send(':');
  uart_send_hex_byte(val);
  uart_send_crlf();

  return val;
}

void paloma_api_param_set_ubyte(u08 slot, u08 val)
{
  test_ubyte = val;

  uart_send_pstring(PSTR("set_ubyte: "));
  uart_send_hex_byte(slot);
  uart_send(':');
  uart_send_hex_byte(val);
  uart_send_crlf();
}

u16 paloma_api_param_get_uword(u08 slot, u08 def)
{
  u16 val = def ? TEST_UWORD_VAL : test_uword;

  uart_send_pstring(PSTR("get_uword: "));
  uart_send_hex_byte(slot);
  uart_send(':');
  uart_send_hex_word(val);
  uart_send_crlf();

  return val;
}

void paloma_api_param_set_uword(u08 slot, u16 val)
{
  test_uword = val;

  uart_send_pstring(PSTR("set_uword: "));
  uart_send_hex_byte(slot);
  uart_send(':');
  uart_send_hex_word(val);
  uart_send_crlf();
}

u32 paloma_api_param_get_ulong(u08 slot, u08 def)
{
  u32 val = def ? TEST_ULONG_VAL : test_ulong;

  uart_send_pstring(PSTR("get_ulong: "));
  uart_send_hex_byte(slot);
  uart_send(':');
  uart_send_hex_long(val);
  uart_send_crlf();

  return val;
}

void paloma_api_param_set_ulong(u08 slot, u32 val)
{
  test_ulong = val;

  uart_send_pstring(PSTR("set_ulong: "));
  uart_send_hex_byte(slot);
  uart_send(':');
  uart_send_hex_long(val);
  uart_send_crlf();
}

u08 paloma_api_param_get_buffer(u08 slot, u08 *data)
{
  uart_send_pstring(PSTR("get_buffer: "));
  uart_send_hex_byte(slot);
  uart_send('#');
  uart_send_hex_byte(test_buf_size);
  uart_send(':');

  memcpy(data, test_buf, test_buf_size);
  return test_buf_size;
}

void paloma_api_param_set_buffer(u08 slot, u08 *data, u08 size)
{
  uart_send_pstring(PSTR("set_buffer: "));
  uart_send_hex_byte(slot);
  uart_send('#');
  uart_send_hex_byte(size);
  uart_send(':');

  memcpy(test_buf, data, size);
  test_buf_size = size;
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
