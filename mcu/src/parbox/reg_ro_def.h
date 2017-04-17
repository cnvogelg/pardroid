#ifndef REG_RO_DEF_H
#define REG_RO_DEF_H

#include "pend.h"
#include "reg_ro.h"

// proto values
extern const u16 ro_version ROM_ATTR;
extern const u16 ro_machtag ROM_ATTR;

extern u16 ro_num_regs(void);

#define REG_RO_PROTO_APPID(appid)   static const u16 ro_fw_id ROM_ATTR = appid;

#define REG_RO_PROTO_DEFAULTS \
  REG_RO_TABLE_ROM_W(ro_version), \
  REG_RO_TABLE_ROM_W(ro_machtag), \
  REG_RO_TABLE_ROM_W(ro_fw_id), \
  REG_RO_TABLE_FUNC(ro_num_regs), \
  REG_RO_TABLE_RAM_W(pend_total),

#endif
