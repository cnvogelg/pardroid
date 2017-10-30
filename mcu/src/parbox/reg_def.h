#ifndef REG_DEF_H
#define REG_DEF_H

#include "reg.h"

// proto values
extern const u16 fw_version ROM_ATTR;
extern const u16 fw_machtag ROM_ATTR;

extern void reg_get_error(u16 *valp, u08 mode);

#define REG_PROTO_APPID(appid)   const u16 fw_id ROM_ATTR = appid;

REG_TABLE_DECLARE(base)

#endif
