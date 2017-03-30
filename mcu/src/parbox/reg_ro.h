#ifndef REG_RO_H
#define REG_RO_H

extern u16 reg_ro_get(u08 reg_num);

#define REG_RO_FLAG_NONE      0
#define REG_RO_FLAG_RAM       1
#define REG_RO_FLAG_BYTE      2
#define REG_RO_FLAG_FUNC      4

struct reg_ro_table_entry {
  u16 ptr;
  u08 flags;
};
typedef struct reg_ro_table_entry reg_ro_table_entry_t;

typedef u16 (*reg_ro_func_t)(void);

extern const u08 reg_ro_table_size ROM_ATTR;
extern const reg_ro_table_entry_t reg_ro_table[] ROM_ATTR;

#define REG_RO_TABLE_SIZE           sizeof(reg_ro_table)/sizeof(reg_ro_table[0])

#define REG_RO_TABLE_BEGIN          const reg_ro_table_entry_t reg_ro_table[] ROM_ATTR = {
#define REG_RO_TABLE_END            }; const u08 reg_ro_table_size = REG_RO_TABLE_SIZE;

#define REG_RO_TABLE_ROM_W(x)       { (u16)&x, REG_RO_FLAG_NONE }
#define REG_RO_TABLE_ROM_W_PTR(x)   { (u16)x,  REG_RO_FLAG_NONE }
#define REG_RO_TABLE_ROM_B(x)       { (u16)&x, REG_RO_FLAG_BYTE }
#define REG_RO_TABLE_ROM_B_PTR(x)   { (u16)x,  REG_RO_FLAG_BYTE }

#define REG_RO_TABLE_RAM_W(x)       { (u16)&x, REG_RO_FLAG_RAM }
#define REG_RO_TABLE_RAM_W_PTR(x)   { (u16)x,  REG_RO_FLAG_RAM }
#define REG_RO_TABLE_RAM_B(x)       { (u16)&x, REG_RO_FLAG_RAM | REG_RO_FLAG_BYTE }
#define REG_RO_TABLE_RAM_B_PTR(x)   { (u16)x,  REG_RO_FLAG_RAM | REG_RO_FLAG_BYTE }

#define REG_RO_TABLE_FUNC(x)        { (u16)x,  REG_RO_FLAG_FUNC }

#endif
