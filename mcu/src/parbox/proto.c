#include "types.h"
#include "proto_low.h"
#include "proto.h"
#include "debug.h"
#include "system.h"

void proto_init(u08 status)
{
  proto_low_init(status);
  DS("init:"); DB(status); DNL;
}

static void action(u08 num)
{
  DS("a:"); DB(num); DNL;
  proto_api_action(num);
}

static void function(u08 num)
{
  DS("f:"); DB(num); DNL;
  proto_api_function(num);
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
  if(cmd == 0xff) {
    // no clock lined pulled -> idle
    return;
  }

  // action=0 is invalid
  // it is generated when amiga is powered off
  if(cmd == 0x10) {
    return;
  }

  DS("cmd:"); DB(cmd); DNL;

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
