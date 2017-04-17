#ifndef REG_RW_DEF_H
#define REG_RW_DEF_H

#include "reg_rw.h"

extern u16 rw_drv_tag;

#define REG_RW_PROTO_DEFAULTS \
  REG_RW_TABLE_RAM_W(rw_drv_tag),

#endif
