#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "proto.h"
#include "proto_shared.h"
#include "reg_rw.h"

int reg_rw_get_drv_tag(proto_handle_t *ph, UWORD *tag)
{
  return proto_reg_rw_read(ph, PROTO_REG_RW_DRV_TAG, tag);
}

int reg_rw_set_drv_tag(proto_handle_t *ph, UWORD tag)
{
  return proto_reg_rw_write(ph, PROTO_REG_RW_DRV_TAG, &tag);
}
