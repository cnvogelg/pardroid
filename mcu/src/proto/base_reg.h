#ifndef BASE_REG_H
#define BASE_REG_H

#include "reg.h"

// proto values
extern const u16 fw_version ROM_ATTR;
extern const u16 fw_machtag ROM_ATTR;

extern void base_reg_get_error(u16 *valp, u08 mode);
extern void base_reg_get_num_regs(u16 *valp, u08 mode);

#define BASE_REG_APPID(appid)   const u16 fw_id ROM_ATTR = appid;

REG_TABLE_DECLARE(base)

#endif
