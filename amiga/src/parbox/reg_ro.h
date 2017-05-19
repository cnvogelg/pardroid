#ifndef REG_RO_H
#define REG_RO_H

#include "proto.h"

extern int reg_ro_get_fw_version(proto_handle_t *ph, UWORD *version);
extern int reg_ro_get_fw_machtag(proto_handle_t *ph, UWORD *machtag);
extern int reg_ro_get_fw_id(proto_handle_t *ph, UWORD *fw_id);
extern int reg_ro_get_num_regs(proto_handle_t *ph, UBYTE *num_ro, UBYTE *num_rw);
extern int reg_ro_get_pend_total(proto_handle_t *ph, UWORD *pend_total);

#endif
