#ifndef REG_H
#define REG_H

#include "proto.h"

extern int reg_get(proto_handle_t *ph, UWORD num, UWORD *val);
extern int reg_set(proto_handle_t *ph, UWORD num, UWORD val);

extern int reg_get_fw_version(proto_handle_t *ph, UWORD *version);
extern int reg_get_fw_machtag(proto_handle_t *ph, UWORD *machtag);
extern int reg_get_fw_id(proto_handle_t *ph, UWORD *fw_id);
extern int reg_get_num_regs(proto_handle_t *ph, UWORD *num_regs);
extern int reg_get_error(proto_handle_t *ph, UWORD *error);

#endif
