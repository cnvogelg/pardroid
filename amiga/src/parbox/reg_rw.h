#ifndef REG_RW_H
#define REG_RW_H

#include "proto.h"

extern int reg_rw_get_drv_tag(proto_handle_t *ph, UWORD *tag);
extern int reg_rw_set_drv_tag(proto_handle_t *ph, UWORD tag);

#endif
