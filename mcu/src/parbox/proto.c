#include "types.h"
#include "proto_low.h"
#include "proto.h"
#include "debug.h"
#include "system.h"

void proto_init(void)
{
  proto_low_init();
}

static void action(u08 num)
{
  DS("a:"); DB(num); DNL;
  proto_api_action(num);
}

static void rw_reg_write(u08 reg)
{
  // master wants to write a u16
  DS("rw:"); DB(reg); DC('=');
  u16 val = proto_low_write_word();
  DW(val);
  proto_api_set_rw_reg(reg, val);
  DC('.'); DNL;
}

static void rw_reg_read(u08 reg)
{
  // master wants to reead a u16
  DS("rr:"); DB(reg); DC('=');
  u16 val = proto_api_get_rw_reg(reg);
  DW(val);
  proto_low_read_word(val);
  DC('.'); DNL;
}

static void ro_reg_read(u08 reg)
{
  // master wants to reead a u16
  DS("or:"); DB(reg); DC('=');
  u16 val = proto_api_get_ro_reg(reg);
  DW(val);
  proto_low_read_word(val);
  DC('.'); DNL;
}

static void msg_read(u08 chan)
{
  DS("mr:"); DB(chan); DC('=');
  u16 size = 0;
  u08 *buf = proto_api_prepare_read_msg(chan, &size);
  DW(size);
  proto_low_read_block(size, buf);
  proto_api_done_read_msg(chan);
  DC('.'); DNL;
}

static void msg_write(u08 chan)
{
  DS("mw:"); DB(chan); DC('=');
  u16 max_size = 0;
  u08 *buf = proto_api_prepare_write_msg(chan, &max_size);
  DW(max_size); DC(':');
  u16 size = proto_low_write_block(max_size, buf);
  DW(size);
  proto_api_done_write_msg(chan, size);
  DC('.'); DNL;
}

void proto_handle(void)
{
  u08 cmd = proto_low_get_cmd();
  switch(cmd) {
    case PROTO_CMD_IDLE:
      // nothing to do for now. return
      break;
    case PROTO_CMD_INVALID:
      //DS("invalid"); DNL;
      break;
    default:
      {
        u08 cmd_base = cmd & PROTO_CMD_MASK;
        u08 num = cmd - cmd_base;
        switch(cmd_base) {
          case PROTO_CMD_ACTION:
            action(num);
            break;
          case PROTO_CMD_RW_REG_WRITE:
            rw_reg_write(num);
            break;
          case PROTO_CMD_RW_REG_READ:
            rw_reg_read(num);
            break;
          case PROTO_CMD_MSG_WRITE:
            msg_write(num);
            break;
          case PROTO_CMD_MSG_READ:
            msg_read(num);
            break;
          case PROTO_CMD_RO_REG_READ:
            ro_reg_read(num);
            break;
          default:
            DS("?:"); DB(cmd); DNL;
            break;
        }
      }
      break;
  }
}
