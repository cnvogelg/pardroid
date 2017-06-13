#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "proto.h"
#include "proto_shared.h"
#include "reg.h"

int reg_get(proto_handle_t *ph, UWORD num, UWORD *val)
{
  // set reg addr
  int res = proto_function_write(ph, PROTO_FUNC_REGADDR_SET, num);
  if(res != PROTO_RET_OK) {
    return res;
  }
  // get value
  res = proto_function_read(ph, PROTO_FUNC_REG_READ, val);
  return res;
}

int reg_set(proto_handle_t *ph, UWORD num, UWORD val)
{
  // set reg addr
  int res = proto_function_write(ph, PROTO_FUNC_REGADDR_SET, num);
  if(res != PROTO_RET_OK) {
    return res;
  }
  // set value
  res = proto_function_write(ph, PROTO_FUNC_REG_WRITE, val);
  return res;
}

int reg_get_fw_version(proto_handle_t *ph, UWORD *version)
{
  return reg_get(ph, PROTO_REG_FW_VERSION, version);
}

int reg_get_fw_machtag(proto_handle_t *ph, UWORD *machtag)
{
  return reg_get(ph, PROTO_REG_FW_MACHTAG, machtag);
}

int reg_get_fw_id(proto_handle_t *ph, UWORD *fw_id)
{
  return reg_get(ph, PROTO_REG_FW_ID, fw_id);
}

int reg_get_num_regs(proto_handle_t *ph, UWORD *num_regs)
{
  return reg_get(ph, PROTO_REG_NUM_REGS, num_regs);
}

int reg_ro_get_pend_total(proto_handle_t *ph, UWORD *pend_total)
{
  return reg_get(ph, PROTO_REG_PEND_TOTAL, pend_total);
}
