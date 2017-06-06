#ifndef REG_H
#define REG_H

extern u16 reg_get(u08 reg_num);
extern void reg_set(u08 reg_num, u16 val);

#define REG_FLAG_NONE      0
#define REG_FLAG_BYTE      1
#define REG_FLAG_FUNC      2
#define REG_FLAG_WRITE     4
#define REG_FLAG_ROM       8

typedef u16 (*reg_get_func_t)(void);
typedef void (*reg_set_func_t)(u16 v);

struct reg_table_entry {
  union {
    u16 *wptr;
    u08 *cptr;
  } ptr;
  reg_get_func_t get_func;
  reg_set_func_t set_func;
  u08 flags;
};
typedef struct reg_table_entry reg_table_entry_t;

extern const u08 reg_table_size ROM_ATTR;
extern const reg_table_entry_t reg_table[] ROM_ATTR;

#define REG_TABLE_SIZE           sizeof(reg_table)/sizeof(reg_table[0])

#define REG_TABLE_BEGIN          const reg_table_entry_t reg_table[] ROM_ATTR = {
#define REG_TABLE_END            }; const u08 reg_table_size ROM_ATTR = REG_TABLE_SIZE;

#define REG_TABLE_RW_RAM_W(v)    { {.wptr = &v}, .flags=REG_FLAG_WRITE }
#define REG_TABLE_RW_RAM_B(v)    { {.cptr = &v}, .flags=REG_FLAG_BYTE|REG_FLAG_WRITE }
#define REG_TABLE_RW_FUNC(g,s)   { .get_func=g, .set_func=s, .flags=REG_FLAG_FUNC|REG_FLAG_WRITE }

#define REG_TABLE_RO_RAM_W(v)    { {.wptr = (u16 *)&v}, .flags=REG_FLAG_NONE }
#define REG_TABLE_RO_RAM_B(v)    { {.cptr = (u08 *)&v}, .flags=REG_FLAG_BYTE }
#define REG_TABLE_RO_ROM_W(v)    { {.wptr = (u16 *)&v}, .flags=REG_FLAG_ROM }
#define REG_TABLE_RO_ROM_B(v)    { {.cptr = (u08 *)&v}, .flags=REG_FLAG_BYTE|REG_FLAG_ROM }
#define REG_TABLE_RO_ROM_W_PTR(v) { {.wptr = (u16 *)v}, .flags=REG_FLAG_ROM }
#define REG_TABLE_RO_ROM_B_PTR(v) { {.cptr = (u08 *)v}, .flags=REG_FLAG_BYTE|REG_FLAG_ROM }
#define REG_TABLE_RO_FUNC(g)     { .get_func=g, .flags=REG_FLAG_FUNC }

#endif
