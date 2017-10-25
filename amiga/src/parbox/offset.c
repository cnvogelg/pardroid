#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "proto.h"
#include "proto_shared.h"
#include "reg.h"

int offset_get(proto_handle_t *ph, UWORD num, ULONG *val)
{
  // set reg addr
  int res = proto_function_write(ph, PROTO_FUNC_OFFSLOT_SET, num);
  if(res != PROTO_RET_OK) {
    return res;
  }
  // get value
  res = proto_function_read_long(ph, PROTO_FUNC_OFFSET_GET, val);
  return res;
}

int offset_set(proto_handle_t *ph, UWORD num, ULONG val)
{
  // set reg addr
  int res = proto_function_write(ph, PROTO_FUNC_OFFSLOT_SET, num);
  if(res != PROTO_RET_OK) {
    return res;
  }
  // set value
  res = proto_function_write_long(ph, PROTO_FUNC_OFFSET_SET, val);
  return res;
}
