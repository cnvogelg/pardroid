#ifndef REG_DEF_H
#define REG_DEF_H

#include "reg.h"

// proto values
extern const u16 fw_version ROM_ATTR;
extern const u16 fw_machtag ROM_ATTR;

extern void reg_get_error(u16 *valp, u08 mode);

#define REG_PROTO_APPID(appid)   static const u16 fw_id ROM_ATTR = appid;

#define REG_TABLE_DEFAULTS \
  REG_TABLE_RO_ROM_W(fw_version), \
  REG_TABLE_RO_ROM_W(fw_machtag), \
  REG_TABLE_RO_ROM_W(fw_id), \
  REG_TABLE_RO_ROM_W(reg_table_size), \
  REG_TABLE_RO_FUNC(reg_get_error), \

#endif
