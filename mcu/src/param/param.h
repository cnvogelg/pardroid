#ifndef PARAM_H
#define PARAM_H

#include "arch.h"

#define PARAM_CHECK_OK              0
#define PARAM_CHECK_WRONG_CRC       1
#define PARAM_CHECK_WRONG_SIZE      2
#define PARAM_CHECK_WRONG_VERSION   3

#define PARAM_TYPE_BYTE             0
#define PARAM_TYPE_WORD             1
#define PARAM_TYPE_LONG             2
#define PARAM_TYPE_MAC_ADDR         3
#define PARAM_TYPE_IP_ADDR          4
#define PARAM_TYPE_STRING           5

#define PARAM_EEP_INVALID           0
#define PARAM_EEP_VALID             1
#define PARAM_EEP_DIRTY             2

struct param_def {
  u08         type;
  u08         size;
  u16         offset;
  rom_pchar   name;
  rom_pchar   def_val;
};
typedef struct param_def param_def_t;
typedef const param_def_t *param_def_ptr_t;

#define PARAM_EEP_OFFSET_CRC        0
#define PARAM_EEP_OFFSET_SIZE       2
#define PARAM_EEP_OFFSET_VERSION    3
#define PARAM_EEP_OFFSET_DATA       4

// param definition
#define PARAM_DEF(t,o,s,n) \
  static const char param_name_ ## n[] ROM_ATTR = #n; \
  static const param_def_t param_def_ ## n ROM_ATTR = \
  { .type=t, .size=s, .offset=o, .name=param_name_ ## n, \
    .def_val = (rom_pchar)&param_def_val_ ## n };

#define PARAM_DEF_VALUE_BYTE(n,v) \
  static const u08 param_def_val_ ## n ROM_ATTR = v;
#define PARAM_DEF_VALUE_WORD(n,v) \
  static const u16 param_def_val_ ## n ROM_ATTR = v;
#define PARAM_DEF_VALUE_LONG(n,v) \
  static const u32 param_def_val_ ## n ROM_ATTR = v;
#define PARAM_DEF_VALUE_BLOCK(n,s) \
  static const u08 param_def_val_ ## n [s] ROM_ATTR

#define PARAM_DEF_TOTAL(n) \
  const u16 param_total_size ROM_ATTR = n;
#define PARAM_DEF_TOTAL_DECLARE \
  extern const u16 param_total_size ROM_ATTR;
#define PARAM_GET_DEF_TOTAL()    read_rom_char(&param_total_size)

#define PARAM_VERSION(v) \
  const u08 param_version ROM_ATTR = v;
#define PARAM_VERSION_DECLARE \
  extern const u08 param_version ROM_ATTR;
#define PARAM_GET_VERSION()      read_rom_char(&param_version)

// param table
#define PARAM_TABLE_DECLARE \
  extern const param_def_ptr_t param_table[] ROM_ATTR; \
  extern const u08 param_table_size ROM_ATTR;
#define PARAM_TABLE_BEGIN   const param_def_ptr_t param_table[] ROM_ATTR = {

#define PARAM_TABLE_END     }; \
  const u08 param_table_size ROM_ATTR = sizeof(param_table) / sizeof(param_def_ptr_t);

#define PARAM_TABLE_ENTRY(n)  &param_def_ ## n

#define PARAM_TABLE_GET_SIZE()   read_rom_char(&param_table_size)

// param funcs
#define PARAM_FUNC_BYTE_DECLARE(n) \
  extern u08  param_get_ ## n (void); \
  extern void param_set_ ## n (u08 val);
#define PARAM_FUNC_BYTE(n) \
  u08  param_get_ ## n (void) { return param_get_byte(&param_def_ ## n); } \
  void param_set_ ## n (u08 val) { param_set_byte(&param_def_ ## n, val); }

#define PARAM_FUNC_WORD_DECLARE(n) \
  extern u16  param_get_ ## n (void); \
  extern void param_set_ ## n (u16 val);
#define PARAM_FUNC_WORD(n) \
  u16  param_get_ ## n (void) { return param_get_word(&param_def_ ## n); } \
  void param_set_ ## n (u16 val) { param_set_word(&param_def_ ## n, val); }

#define PARAM_FUNC_LONG_DECLARE(n) \
  extern u32  param_get_ ## n (void); \
  extern void param_set_ ## n (u32 val);
#define PARAM_FUNC_LONG(n) \
  u32  param_get_ ## n (void) { return param_get_long(&param_def_ ## n); } \
  void param_set_ ## n (u32 val) { param_set_long(&param_def_ ## n, val); }

#define PARAM_FUNC_BLOCK_DECLARE(n,s) \
  extern void param_get_ ## n (u08 data[s]); \
  extern void param_set_ ## n (const u08 data[s]);
#define PARAM_FUNC_BLOCK(n,s) \
  void param_get_ ## n (u08 data[s]) { return param_get_block(&param_def_ ## n, data); } \
  void param_set_ ## n (const u08 data[s]) { param_set_block(&param_def_ ## n, data); }

// generic API
extern void param_init(void);
extern u08  param_check(void);
extern void param_sync(void);
extern void param_reset(void);
extern u08  param_is_eep_valid(void);
extern void param_dump(void);

extern u08  param_get_byte_default(param_def_ptr_t def);
extern u08  param_get_byte(param_def_ptr_t def);
extern void param_set_byte(param_def_ptr_t def, u08 val);

extern u16  param_get_word_default(param_def_ptr_t def);
extern u16  param_get_word(param_def_ptr_t def);
extern void param_set_word(param_def_ptr_t def, u16 val);

extern u32  param_get_long_default(param_def_ptr_t def);
extern u32  param_get_long(param_def_ptr_t def);
extern void param_set_long(param_def_ptr_t def, u32 val);

extern void param_get_block_default(param_def_ptr_t def, u08 *data);
extern void param_get_block(param_def_ptr_t def, u08 *data);
extern void param_set_block(param_def_ptr_t def, const u08 *data);

#endif
