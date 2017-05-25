#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "proto.h"
#include "proto_shared.h"
#include "reg_ro.h"

int reg_ro_get_fw_version(proto_handle_t *ph, UWORD *version)
{
  return proto_reg_ro_read(ph, PROTO_REG_RO_FW_VERSION, version);
}

int reg_ro_get_fw_machtag(proto_handle_t *ph, UWORD *machtag)
{
  return proto_reg_ro_read(ph, PROTO_REG_RO_FW_MACHTAG, machtag);
}

int reg_ro_get_fw_id(proto_handle_t *ph, UWORD *fw_id)
{
  return proto_reg_ro_read(ph, PROTO_REG_RO_FW_ID, fw_id);
}

int reg_ro_get_num_regs(proto_handle_t *ph, UBYTE *num_ro, UBYTE *num_rw)
{
  UWORD w;
  int res = proto_reg_ro_read(ph, PROTO_REG_RO_NUM_REGS, &w);
  *num_ro = (UBYTE)(w >> 8);
  *num_rw = (UBYTE)(w & 0xff);
  return res;
}

int reg_ro_get_pend_total(proto_handle_t *ph, UWORD *pend_total)
{
  return proto_reg_ro_read(ph, PROTO_REG_RO_PEND_TOTAL, pend_total);
}
