#ifndef REG_DEF_H
#define REG_DEF_H

#include "reg.h"

// proto values
extern const u16 fw_version ROM_ATTR;
extern const u16 fw_machtag ROM_ATTR;

extern u16 get_num_regs(void);

#define REG_PROTO_APPID(appid)   static const u16 fw_id ROM_ATTR = appid;

#define REG_TABLE_DEFAULTS \
  REG_TABLE_RO_ROM_W(fw_version), \
  REG_TABLE_RO_ROM_W(fw_machtag), \
  REG_TABLE_RO_ROM_W(fw_id), \
  REG_TABLE_RO_ROM_B(reg_table_size), \

#endif
