#include "types.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_PROTO

#include "proto_low.h"
#include "proto.h"
#include "debug.h"
#include "system.h"
#include "timer.h"
#include "status.h"

static u16 msg_size;

void proto_init(void)
{
  proto_low_init();
  DC('I');
}

void proto_first(void)
{
  // expect reset command
  u08 cmd = proto_low_get_cmd();
  if(cmd != PROTO_CMD_ACTION_RESET && cmd != PROTO_CMD_ACTION_BOOTLOADER) {
    DC('!'); DB(cmd); DNL;
    system_sys_reset();
  }
  // ack action
  DC('{'); DB(cmd);
  proto_low_action();
  proto_low_end();
  DC('}');
}

void proto_trigger_signal(void)
{
  // trigger ack irq at Amiga
  DC('#');
  proto_low_ack_lo();
  timer_delay_1us();
  proto_low_ack_hi();
}

static void handle_action(u08 num)
{
  // some actions need special handling:
  // immediate reset
  if(num == PROTO_ACTION_RESET || num == PROTO_ACTION_BOOTLOADER) {
    DC('r');
    system_sys_reset();
  }

  proto_low_action();
  if(num >= PROTO_ACTION_USER) {
    proto_api_action(num);
  }
  proto_low_end();

  // knok resets after action
  if(num == PROTO_ACTION_KNOK) {
    DC('k');
    system_sys_reset();
  }
}

static void handle_wfunc_read(u08 num)
{
  u16 val = proto_api_wfunc_read(num);
  proto_low_read_word(val);
  proto_low_end();
}

static void handle_wfunc_write(u08 num)
{
  u16 val = proto_low_write_word();
  proto_api_wfunc_write(num, val);
  proto_low_end();
}

static void handle_lfunc_read(u08 num)
{
  u32 val = proto_api_lfunc_read(num);
  proto_low_read_long(val);
  proto_low_end();
}

static void handle_lfunc_write(u08 num)
{
  u32 val = proto_low_write_long();
  proto_api_lfunc_write(num, val);
  proto_low_end();
}

static void handle_read_offset(u08 chan)
{
  u32 off = proto_api_read_offset(chan);
  DL(off); DNL;
  proto_low_read_long(off);
  proto_low_end();
}

static void handle_write_offset(u08 chan)
{
  u32 off = proto_low_write_long();
  proto_api_write_offset(chan, off);
  proto_low_end();
  DL(off); DNL;
}

static void handle_read_mtu(u08 chan)
{
  u16 mtu = proto_api_read_mtu(chan);
  DW(mtu); DNL;
  proto_low_read_word(mtu);
  proto_low_end();
}

static void handle_write_mtu(u08 chan)
{
  u16 mtu = proto_low_write_word();
  proto_api_write_mtu(chan, mtu);
  proto_low_end();
  DW(mtu); DNL;
}

static void handle_msg_read_size(u08 chan)
{
  u16 size = proto_api_read_msg_size(chan);
  DW(size); DNL;

  // send size
  proto_low_read_word(size);
  proto_low_end();

  // store size for subsequent msg_read_data call
  msg_size = size;
}

static void handle_msg_write_size(u08 chan)
{
  // recv size
  u16 size = proto_low_write_word();
  proto_low_end();

  // report size and receive msg size
  msg_size = proto_api_write_msg_size(chan, size);
  DW(size); DNL;
}

static void handle_msg_read_data(u08 chan)
{
  // ignore request if size is empty
  if(msg_size == 0) {
    return;
  }

  // get filled buffer and send it
  u08 *buf = proto_api_read_msg_begin(chan, msg_size);
  if(buf == NULL) {
    DC('#');
    proto_api_read_block_spi(msg_size);
  } else {
    proto_low_read_block(msg_size, buf);
  }
  proto_api_read_msg_done(chan, msg_size);

  proto_low_end();
}

static void handle_msg_write_data(u08 chan)
{
  // ignore request if size is empty
  if(msg_size == 0) {
    return;
  }

  // get buffer and fill it
  u08 *buf = proto_api_write_msg_begin(chan, msg_size);
  if(buf == NULL) {
    DC('#');
    proto_api_write_block_spi(msg_size);
  } else {
    proto_low_write_block(msg_size, buf);
  }
  proto_api_write_msg_done(chan, msg_size);

  proto_low_end();
}

void proto_handle(void)
{
  // read command from bits 0..4 in idle byte
  u08 cmd = proto_low_get_cmd();
  if(cmd == 0xff) {
    // no clock lined pulled -> idle
    return;
  }

  DC('['); DB(cmd);

  // extract command group
  u08 grp = cmd & PROTO_CMD_MASK;
  u08 chn = cmd & PROTO_CMD_ARG;
  switch(grp) {
    case PROTO_CMD_ACTION:
      DC('A');
      handle_action(chn);
      break;
    case PROTO_CMD_WFUNC_READ:
      DC('w');
      handle_wfunc_read(chn);
      break;
    case PROTO_CMD_WFUNC_WRITE:
      DC('W');
      handle_wfunc_write(chn);
      break;
    case PROTO_CMD_LFUNC_READ:
      DC('l');
      handle_lfunc_read(chn);
      break;
    case PROTO_CMD_LFUNC_WRITE:
      DC('L');
      handle_lfunc_write(chn);
      break;
    case PROTO_CMD_MSG_READ_DATA:
      DC('m');
      handle_msg_read_data(chn);
      break;
    case PROTO_CMD_MSG_WRITE_DATA:
      DC('M');
      handle_msg_write_data(chn);
      break;
    case PROTO_CMD_MSG_READ_SIZE:
      DC('s');
      handle_msg_read_size(chn);
      break;
    case PROTO_CMD_MSG_WRITE_SIZE:
      DC('S');
      handle_msg_write_size(chn);
      break;
    case PROTO_CMD_READ_OFFSET:
      DC('o');
      handle_read_offset(chn);
      break;
    case PROTO_CMD_WRITE_OFFSET:
      DC('O');
      handle_write_offset(chn);
      break;
    case PROTO_CMD_READ_MTU:
      DC('u');
      handle_read_mtu(chn);
      break;
    case PROTO_CMD_WRITE_MTU:
      DC('U');
      handle_write_mtu(chn);
      break;
    default:
      DC('!'); DNL;
      // trigger a reset to re-enter knok
      system_sys_reset();
      break;
  }

  DC(']'); DB(cmd); DNL;
}
