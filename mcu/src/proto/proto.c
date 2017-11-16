#include "types.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_PROTO

#include "proto_low.h"
#include "proto.h"
#include "debug.h"
#include "system.h"

static u08 in_cmd = 0xff;

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
  DS("mr:#"); DB(chan); DC(':');
  u16 size_bytes = 0;
  u08 *buf = proto_api_read_msg_prepare(chan, &size_bytes);
  u16 chn_ext = chan | 0x4200;
  DC('+'); DW(size_bytes); DC('%'); DW(chn_ext);
  u16 size_words = size_bytes >> 1;
  u08 res = proto_low_read_block(size_words, buf, chn_ext);
  DC('>'); DB(res);
  proto_api_read_msg_done(chan, res);
  DC('.'); DNL;
}

static void msg_write(u08 chan)
{
  DS("mw:#"); DB(chan); DC(':');
  u16 max_bytes = 0;
  u08 *buf = proto_api_write_msg_prepare(chan, &max_bytes);
  DW(max_bytes); DC(':');
  u16 chn_ext = 0;
  u16 max_words = max_bytes >> 1;
  u16 size_words = proto_low_write_block(max_words, buf, &chn_ext);
  u16 size_bytes = size_words << 1;
  DC('+'); DW(size_bytes); DC('%'); DW(chn_ext);
  proto_api_write_msg_done(chan, size_bytes);
  DC('.'); DNL;
}

u08 proto_current_cmd(void)
{
  return in_cmd;
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

  in_cmd = cmd;

  // extract command group
  u08 grp = cmd & PROTO_CMD_MASK;
  u08 chn = cmd & PROTO_CMD_SUB_MASK;
  switch(grp) {
    case PROTO_CMD_MSG_WRITE:
      msg_write(chn);
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

  in_cmd = 0xff;
}
