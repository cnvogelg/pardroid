#ifndef REG_H
#define REG_H

#define REG_FLAG_NONE      0
#define REG_FLAG_FUNC      1
#define REG_FLAG_WRITE     2
#define REG_FLAG_ROM       4

#define REG_MODE_READ      0
#define REG_MODE_WRITE     1

typedef void (*reg_func_t)(u16 *v,u08 mode);

struct reg_def {
  union {
    void        *var;
    reg_func_t  func;
  } ptr;
  u08 flags;
};
typedef struct reg_def reg_def_t;

struct reg_table {
  const struct reg_table  * const next;
  u08  offset;
  u08  size;
  const reg_def_t * const entries;
};
typedef struct reg_table reg_table_t;

#define REG_TABLE_DECLARE(name)  \
    extern const reg_table_t reg_table_ ## name ROM_ATTR;

#define REG_TABLE_BEGIN(name)  \
    static const reg_def_t reg_defs_ ## name[] ROM_ATTR = { \

#define REG_TABLE_END(name, off, nxt)  \
    }; \
    const reg_table_t reg_table_ ## name ROM_ATTR = { \
      .next = nxt, \
      .offset = off, \
      .size = sizeof(reg_defs_ ## name) / sizeof(reg_def_t), \
      .entries = reg_defs_ ## name \
    };

#define REG_TABLE_SETUP(name)  \
    const reg_table_t * const reg_table ROM_ATTR = &reg_table_ ## name;

#define REG_TABLE_REF(name)  \
    &reg_table_ ## name

#define REG_TABLE_RW_RAM_W(v)    { {.var = (void *)(&v)}, .flags=REG_FLAG_WRITE }
#define REG_TABLE_RW_FUNC(f)     { {.func= f},   .flags=REG_FLAG_FUNC|REG_FLAG_WRITE }

#define REG_TABLE_RO_RAM_W(v)    { {.var = (void *)(&v)}, .flags=REG_FLAG_NONE }
#define REG_TABLE_RO_ROM_W(v)    { {.var = (void *)(&v)}, .flags=REG_FLAG_ROM }
#define REG_TABLE_RO_ROM_W_PTR(v) { {.var= (void *)(v)}, .flags=REG_FLAG_ROM }
#define REG_TABLE_RO_FUNC(f)     { {.func= f},   .flags=REG_FLAG_FUNC }

extern u16 reg_get(u08 reg_num);
extern void reg_set(u08 reg_num, u16 val);

#endif
