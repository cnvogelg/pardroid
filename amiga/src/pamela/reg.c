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
  int res = proto_function_write_word(ph, PROTO_WFUNC_REG_ADDR, num);
  if(res != PROTO_RET_OK) {
    return res;
  }
  // get value
  res = proto_function_read_word(ph, PROTO_WFUNC_REG_VALUE, val);
  return res;
}

int reg_set(proto_handle_t *ph, UWORD num, UWORD val)
{
  // set reg addr
  int res = proto_function_write_word(ph, PROTO_WFUNC_REG_ADDR, num);
  if(res != PROTO_RET_OK) {
    return res;
  }
  // set value
  res = proto_function_write_word(ph, PROTO_WFUNC_REG_VALUE, val);
  return res;
}

int reg_global_get(proto_handle_t *ph, UBYTE reg, UWORD *val)
{
  UWORD num = PROTO_REG_RANGE_GLOBAL << 8 | reg;
  return reg_get(ph, num, val);
}

int reg_global_set(proto_handle_t *ph, UBYTE reg, UWORD val)
{
  UWORD num = PROTO_REG_RANGE_GLOBAL << 8 | reg;
  return reg_set(ph, num, val);
}

// global registers

int reg_global_get_magic(proto_handle_t *ph, UWORD *magic)
{
  return reg_global_get(ph, PROTO_REG_GLOBAL_MAGIC, magic);
}

int reg_global_get_machtag(proto_handle_t *ph, UWORD *machtag)
{
  return reg_global_get(ph, PROTO_REG_GLOBAL_MACHTAG, machtag);
}

int reg_global_get_fw_id(proto_handle_t *ph, UWORD *fw_id)
{
  return reg_global_get(ph, PROTO_REG_GLOBAL_FW_ID, fw_id);
}

int reg_global_get_fw_version(proto_handle_t *ph, UWORD *fw_version)
{
  return reg_global_get(ph, PROTO_REG_GLOBAL_FW_VERSION, fw_version);
}
