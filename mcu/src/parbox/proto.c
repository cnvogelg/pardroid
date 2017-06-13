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

static void reg_write(void)
{
  // master wants to write a u16
  DS("rw:");
  u16 val;
  u08 reg = proto_low_write_word(&val);
  DB(reg); DC('='); DW(val);
  proto_api_set_reg(reg, val);
  DC('.'); DNL;
}

static void reg_read(void)
{
  // master wants to reead a u16
  DS("rr:");
  // TBD reg addr!
  u16 val = proto_api_get_reg(0);
  DW(val);
  u08 reg2 = proto_low_read_word(val);
  DC(':'); DB(reg2); DC('.'); DNL;
}

static void function(u08 num)
{
  DS("f:"); DB(num); DNL;
  switch(num) {
    case PROTO_FUNC_REG_WRITE:
      reg_write();
      break;
    case PROTO_FUNC_REG_READ:
      reg_read();
      break;
    default:
      DS("?:"); DB(cmd); DNL;
      break;
  }
}

static void msg_read(u08 chan)
{
  DS("mr:"); DB(chan); DC('=');
  u16 size = 0;
  u08 *buf = proto_api_read_msg_prepare(chan, &size);
  u16 chn_ext = chan | 0x4200;
  DW(size); DC(','); DW(chn_ext);
  proto_low_read_block(size, buf, chn_ext);
  proto_api_read_msg_done(chan);
  DC('.'); DNL;
}

static void msg_write(u08 chan)
{
  DS("mw:"); DB(chan); DC('=');
  u16 max_size = 0;
  u08 *buf = proto_api_write_msg_prepare(chan, &max_size);
  DW(max_size); DC(':');
  u16 chn_ext = 0;
  u16 size = proto_low_write_block(max_size, buf, &chn_ext);
  DW(size); DC(','); DW(chn_ext);
  proto_api_write_msg_done(chan, size);
  DC('.'); DNL;
}

void proto_handle(void)
{
  // read command from bits 0..4 in idle byte
  u08 cmd = proto_low_get_cmd();
  if(cmd == 0) {
    // no command set or no clk line pulled -> idle nothing to do
    return;
  }

  // extract command group
  u08 grp = cmd & PROTO_CMD_MASK;
  u08 chn = cmd & PROTO_CMD_SUB_MASK;
  switch(grp) {
    case PROTO_CMD_MSG_WRITE:
      // only accept write commands if no read is pending
      if(!proto_api_read_is_pending()) {
        msg_write(chn);
      } else {
        DS("mw!"); DNL;
      }
      break;
    case PROTO_CMD_MSG_READ:
      msg_read(chn);
      break;
    case PROTO_CMD_ACTION:
      action(chn);
      break;
    case PROTO_CMD_FUNCTION:
      function(chn);
      break;
  }
}
