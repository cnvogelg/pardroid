#ifndef REG_H
#define REG_H

extern u16 reg_get(u08 reg_num);
extern void reg_set(u08 reg_num, u16 val);

#define REG_FLAG_NONE      0
#define REG_FLAG_FUNC      1
#define REG_FLAG_WRITE     2
#define REG_FLAG_ROM       4

#define REG_MODE_READ      0
#define REG_MODE_WRITE     1

typedef void (*reg_func_t)(u16 *v,u08 mode);

struct reg_table_entry {
  union {
    void        *var;
    reg_func_t  func;
  } ptr;
  u08 flags;
};
typedef struct reg_table_entry reg_table_entry_t;

extern const u16 reg_table_size ROM_ATTR;
extern const reg_table_entry_t reg_table[] ROM_ATTR;

#define REG_TABLE_SIZE           sizeof(reg_table)/sizeof(reg_table[0])

#define REG_TABLE_BEGIN          const reg_table_entry_t reg_table[] ROM_ATTR = {
#define REG_TABLE_END            }; const u16 reg_table_size ROM_ATTR = REG_TABLE_SIZE;

#define REG_TABLE_RW_RAM_W(v)    { {.var = (void *)(&v)}, .flags=REG_FLAG_WRITE }
#define REG_TABLE_RW_FUNC(f)     { {.func= f},   .flags=REG_FLAG_FUNC|REG_FLAG_WRITE }

#define REG_TABLE_RO_RAM_W(v)    { {.var = (void *)(&v)}, .flags=REG_FLAG_NONE }
#define REG_TABLE_RO_ROM_W(v)    { {.var = (void *)(&v)}, .flags=REG_FLAG_ROM }
#define REG_TABLE_RO_ROM_W_PTR(v) { {.var= (void *)(v)}, .flags=REG_FLAG_ROM }
#define REG_TABLE_RO_FUNC(f)     { {.func= f},   .flags=REG_FLAG_FUNC }

#endif
