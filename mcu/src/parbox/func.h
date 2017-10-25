#ifndef FUNC_H
#define FUNC_H

typedef void (*func_func_t)(u16 *val);

#define FUNC_FLAG_NONE            0
#define FUNC_FLAG_SET             1

struct func_table_entry {
  func_func_t    func;
  u08            flags;
};
typedef struct func_table_entry func_table_entry_t;

extern const u08 func_table_size ROM_ATTR;
extern const func_table_entry_t func_table[] ROM_ATTR;

#define FUNC_TABLE_SIZE           sizeof(func_table)/sizeof(func_table[0])

#define FUNC_TABLE_BEGIN          const func_table_entry_t func_table[] ROM_ATTR = {
#define FUNC_TABLE_END            }; const u08 func_table_size = FUNC_TABLE_SIZE;

#define FUNC_TABLE_GET_FUNC(x)    { .func = x, .flags = FUNC_FLAG_NONE }
#define FUNC_TABLE_SET_FUNC(x)    { .func = x, .flags = FUNC_FLAG_SET }

#define FUNC_PROTO_BOOTLOADER \
  FUNC_TABLE_GET_FUNC(func_regaddr_get), \
  FUNC_TABLE_SET_FUNC(func_regaddr_set), \
  FUNC_TABLE_GET_FUNC(func_reg_read), \
  FUNC_TABLE_SET_FUNC(func_reg_write),

extern void func_handle(u08 num);

extern void func_regaddr_set(u16 *valp);
extern void func_regaddr_get(u16 *valp);
extern void func_reg_write(u16 *valp);
extern void func_reg_read(u16 *valp);

// ----- external API for register access -----
extern void func_api_set_reg(u08 reg,u16 val);
extern u16  func_api_get_reg(u08 reg);

#endif
