#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG CONFIG_DEBUG_HANDLER

#include "debug.h"

#include "proto_shared.h"
#include "handler.h"
#include "reg.h"
#include "reg_def.h"
#include "handler_reg.h"

static u08 chn_idx;

REG_TABLE_BEGIN(handler)
  REG_TABLE_RW_FUNC(handler_reg_index),
  REG_TABLE_RW_FUNC(handler_reg_ctrl_status),
  REG_TABLE_RW_FUNC(handler_reg_mtu)
REG_TABLE_END(handler, PROTO_REGOFFSET_HANDLER, REG_TABLE_REF(base))

void handler_reg_init(void)
{
  chn_idx = 0;
}

/* set current channel index */
void handler_reg_index(u16 *v,u08 mode)
{
  if(mode == REG_MODE_READ) {
    *v = chn_idx;
  } else {
    u08 max = HANDLER_GET_TABLE_SIZE();
    u08 val = (u08)*v;
    if(val < max) {
      chn_idx = val;
    }
  }
}

/* control/status register */
void handler_reg_ctrl_status(u16 *v,u08 mode)
{
  handler_data_t *data = HANDLER_GET_DATA(chn_idx);

  if(mode == REG_MODE_READ) {
    /* read: status */
    *v = data->flags << 8 | data->status;
  } else {
    /* write: control commands */
    u08 cmd = (u08)*v;
    switch(cmd) {
      case HANDLER_REG_CONTROL_OPEN:
        handler_open(chn_idx);
        break;
      case HANDLER_REG_CONTROL_CLOSE:
        handler_close(chn_idx);
        break;
    }
  }
}

/* get/set current channel MTU */
void handler_reg_mtu(u16 *v,u08 mode)
{
  handler_data_t *data = HANDLER_GET_DATA(chn_idx);

  if(mode == REG_MODE_READ) {
    *v = data->mtu;
  } else {
    u16 mtu_min;
    u16 mtu_max;
    handler_get_mtu(chn_idx, &mtu_max, &mtu_min);
    u16 val = *v;
    if((val >= mtu_min) && (val <= mtu_max)) {
      data->mtu = val;
    }
  }
}

