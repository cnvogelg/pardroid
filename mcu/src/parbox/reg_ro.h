#ifndef REG_RO_H
#define REG_RO_H

#define reg_ro_get  proto_api_get_ro_reg

extern u16 reg_ro_get(u08 num);

// define this in your code:
extern const u08 reg_ro_ptr_table_size ROM_ATTR;
extern const rom_pchar reg_ro_ptr_table[] ROM_ATTR;

#define REG_RO_TABLE_BEGIN          const rom_pchar reg_ro_ptr_table[] ROM_ATTR = {
#define REG_RO_TABLE_END            }; const u08 reg_ro_ptr_table_size = sizeof(reg_ro_ptr_table)/sizeof(rom_pchar);
#define REG_RO_TABLE_ENTRY(x)       (rom_pchar)&x
#define REG_RO_TABLE_ENTRY_ADDR(x)  (rom_pchar)x

#endif
