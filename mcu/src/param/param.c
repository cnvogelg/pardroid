#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG CONFIG_DEBUG_PARAM

#include "debug.h"

#include "param.h"
#include "crc.h"
#include "uart.h"
#include "uartutil.h"

// version and size as defined in ROM
PARAM_TABLE_DECLARE
PARAM_VERSION_DECLARE
PARAM_DEF_TOTAL_DECLARE

// state of param
static u08 eep_state = PARAM_EEP_INVALID;

#define EEP_CRC         (uint16_t *)PARAM_EEP_OFFSET_CRC
#define EEP_SIZE        (uint16_t *)PARAM_EEP_OFFSET_SIZE
#define EEP_VERSION     (uint8_t *)PARAM_EEP_OFFSET_VERSION
#define EEP_DATA        (uint8_t *)PARAM_EEP_OFFSET_DATA
#define EEP_DATA_OFF(o) (uint8_t *)(PARAM_EEP_OFFSET_DATA + o)

void param_init(void)
{
  DS("param: init ");
  u08 ok = param_check();
  DB(ok);
  if(ok != PARAM_CHECK_OK) {
    DS(" reset!");
    param_reset();
  }
  DNL;
}

static u16 calc_eep_crc(u16 my_size)
{
  /* calc crc */
  u16 my_crc = 0;
  const uint8_t *ptr = EEP_DATA;
  for(u16 i=0;i<my_size;i++) {
    u08 val = eeprom_read_byte(ptr);
    my_crc = crc_xmodem_update(my_crc, val);
    ptr++;
  }
  return my_crc;
}

u08 param_check(void)
{
  eep_state = PARAM_EEP_INVALID;

  u16 crc = eeprom_read_word(EEP_CRC);
  u16 size = eeprom_read_word(EEP_SIZE);
  u08 version = eeprom_read_byte(EEP_VERSION);

  DS("crc="); DW(crc); DS(",size="); DW(size); DS(",version="); DB(version); DNL;

  u16 my_size = PARAM_GET_DEF_TOTAL();
  DS("mysize="); DW(my_size); DNL;
  if(size != my_size) {
    return PARAM_CHECK_WRONG_SIZE;
  }

  u08 my_version = PARAM_GET_VERSION();
  if(version != my_version) {
    return PARAM_CHECK_WRONG_VERSION;
  }

  u16 my_crc = calc_eep_crc(my_size);
  if(crc != my_crc) {
    return PARAM_CHECK_WRONG_CRC;
  }

  eep_state = PARAM_EEP_VALID;
  return PARAM_CHECK_OK;
}

void param_sync(void)
{
  u08 my_version = read_rom_char(&param_version);
  u16 my_size = read_rom_word(&param_total_size);
  u16 my_crc = calc_eep_crc(my_size);
  DS("sync my_size="); DW(my_size); DNL;
  eeprom_write_word(EEP_CRC, my_crc);
  eeprom_write_word(EEP_SIZE, my_size);
  eeprom_write_byte(EEP_VERSION, my_version);
  eep_state = PARAM_EEP_VALID;
}

void param_reset(void)
{
  u08 ts = PARAM_TABLE_GET_SIZE();
  for(u08 i=0;i<ts;i++) {
    const param_def_t *def = (const param_def_t *)read_rom_rom_ptr(&param_table[i]);
    u08 size = read_rom_char(&def->size);
    u16 offset = read_rom_word(&def->offset);
    rom_pchar rom_data = read_rom_rom_ptr(&def->def_val);
    uint8_t *eep_data = EEP_DATA_OFF(offset);
    for(u08 s=0;s<size;s++) {
      eeprom_write_byte(eep_data, read_rom_char(rom_data));
      eep_data++;
      rom_data++;
    }
  }
  param_sync();
  eep_state = PARAM_EEP_VALID;
}

u08  param_is_eep_valid(void)
{
  return eep_state != PARAM_EEP_INVALID;
}

static void dump_block(u08 size, u16 offset, u08 sep)
{
  uint8_t *eep_data = EEP_DATA_OFF(offset);
  for(u08 i=0;i<size;i++) {
    u08 val = eeprom_read_byte(eep_data);
    eep_data++;
    uart_send_hex_byte(val);
    if(i<(size-1)) {
      uart_send(sep);
    }
  }
}

void param_dump(void)
{
  uart_send_pstring(PSTR("param:"));
  uart_send_crlf();
  u08 ts = PARAM_TABLE_GET_SIZE();
  for(u08 i=0;i<ts;i++) {
    const param_def_t *def = (const param_def_t *)read_rom_rom_ptr(&param_table[i]);
    rom_pchar name = read_rom_rom_ptr(&def->name);
    u08 type = read_rom_char(&def->type);
    u08 size = read_rom_char(&def->size);
    u16 offset = read_rom_word(&def->offset);

    uart_send_pstring(name);
    uart_send_pstring(PSTR(": ["));
    uart_send_hex_byte(type);
    uart_send(',');
    uart_send_hex_byte(size);
    uart_send('+');
    uart_send_hex_word(offset);
    uart_send_pstring(PSTR("] = "));

    switch(type) {
      case PARAM_TYPE_BYTE:
        {
          u08 val = param_get_byte(def);
          uart_send_hex_byte(val);
          break;
        }
      case PARAM_TYPE_WORD:
        {
          u16 val = param_get_word(def);
          uart_send_hex_word(val);
          break;
        }
      case PARAM_TYPE_LONG:
        {
          u32 val = param_get_long(def);
          uart_send_hex_long(val);
          break;
        }
      case PARAM_TYPE_MAC_ADDR:
        {
          dump_block(size, offset, ':');
          break;
        }
      case PARAM_TYPE_IP_ADDR:
        {
          dump_block(size, offset, '.');
          break;
        }
      case PARAM_TYPE_STRING:
        {
          uint8_t *eep_data = EEP_DATA_OFF(offset);
          while(1) {
            u08 v = eeprom_read_byte(eep_data);
            eep_data++;
            if(v == 0) {
              break;
            }
            uart_send(v);
          }
          break;
        }
      default:
        {
          dump_block(size, offset, ' ');
          break;
        }
    }
    uart_send_crlf();
  }
}

// ----- byte -----
u08  param_get_byte_default(param_def_ptr_t def)
{
  rom_pchar rom_data = read_rom_rom_ptr(&def->def_val);
  return read_rom_char(rom_data);
}

u08  param_get_byte(param_def_ptr_t def)
{
  if(eep_state == PARAM_EEP_INVALID) {
    return param_get_byte_default(def);
  }
  else {
    u16 offset = read_rom_word(&def->offset);
    return eeprom_read_byte(EEP_DATA_OFF(offset));
  }
}

void param_set_byte(param_def_ptr_t def, u08 val)
{
  u16 offset = read_rom_word(&def->offset);
  eeprom_write_byte(EEP_DATA_OFF(offset), val);
  if(eep_state == PARAM_EEP_VALID) {
    eep_state = PARAM_EEP_DIRTY;
  }
}

// ----- word -----
u16  param_get_word_default(param_def_ptr_t def)
{
  rom_pchar rom_data = read_rom_rom_ptr(&def->def_val);
  return read_rom_word(rom_data);
}

u16  param_get_word(param_def_ptr_t def)
{
  if(eep_state == PARAM_EEP_INVALID) {
    return param_get_word_default(def);
  }
  else {
    u16 offset = read_rom_word(&def->offset);
    uint16_t *ptr = (uint16_t *)EEP_DATA_OFF(offset);
    return eeprom_read_word(ptr);
  }
}

void param_set_word(param_def_ptr_t def, u16 val)
{
  u16 offset = read_rom_word(&def->offset);
  uint16_t *ptr = (uint16_t *)EEP_DATA_OFF(offset);
  eeprom_write_word(ptr, val);
  if(eep_state == PARAM_EEP_VALID) {
    eep_state = PARAM_EEP_DIRTY;
  }
}

// ----- long -----
u32  param_get_long_default(param_def_ptr_t def)
{
  rom_pchar rom_data = read_rom_rom_ptr(&def->def_val);
  return (u32)read_rom_word(rom_data) << 16 | (u32)read_rom_word(rom_data+2);
}

u32  param_get_long(param_def_ptr_t def)
{
  if(eep_state == PARAM_EEP_INVALID) {
    return param_get_long_default(def);
  }
  else {
    u16 offset = read_rom_word(&def->offset);
    uint32_t *ptr = (uint32_t *)EEP_DATA_OFF(offset);
    return eeprom_read_dword(ptr);
  }
}

void param_set_long(param_def_ptr_t def, u32 val)
{
  u16 offset = read_rom_word(&def->offset);
  uint32_t *ptr = (uint32_t *)EEP_DATA_OFF(offset);
  eeprom_write_dword(ptr, val);
  if(eep_state == PARAM_EEP_VALID) {
    eep_state = PARAM_EEP_DIRTY;
  }
}

// ----- block -----
void param_get_block_default(param_def_ptr_t def, u08 *data)
{
  rom_pchar rom_data = read_rom_rom_ptr(&def->def_val);
  u08 size = read_rom_char(&def->size);
  for(u08 i=0;i<size;i++) {
    data[i] = read_rom_char(rom_data);
    rom_data++;
  }
}

void param_get_block(param_def_ptr_t def, u08 *data)
{
  u16 offset = read_rom_word(&def->offset);
  u08 size = read_rom_char(&def->size);
  uint8_t *eep_data = EEP_DATA_OFF(offset);
  eeprom_read_block(data, eep_data, size);
}

void param_set_block(param_def_ptr_t def, const u08 *data)
{
  u16 offset = read_rom_word(&def->offset);
  u08 size = read_rom_char(&def->size);
  uint8_t *eep_data = EEP_DATA_OFF(offset);
  eeprom_write_block(data, eep_data, size);
}
