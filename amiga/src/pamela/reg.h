#ifndef REG_H
#define REG_H

#include "proto.h"

extern int reg_get(proto_handle_t *ph, UWORD num, UWORD *val);
extern int reg_set(proto_handle_t *ph, UWORD num, UWORD val);

/* global registers */
extern int reg_global_get_magic(proto_handle_t *ph, UWORD *magic);
extern int reg_global_get_machtag(proto_handle_t *ph, UWORD *machtag);
extern int reg_global_get_fw_version(proto_handle_t *ph, UWORD *version);
extern int reg_global_get_fw_id(proto_handle_t *ph, UWORD *fw_id);

#endif
