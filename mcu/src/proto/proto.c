#include "types.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_PROTO

#include "proto_low.h"
#include "proto_api.h"
#include "proto.h"
#include "debug.h"
#include "system.h"
#include "timer.h"
#include "status.h"

static u08 busy;

void proto_init(void)
{
  proto_low_init();
  DC('I');
}

void proto_first_cmd(void)
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

void proto_set_busy(void)
{
  DC('B');
  if(busy == 0) {
    proto_low_busy_hi();
  }
  busy++;
}

void proto_clr_busy(void)
{
  DC('b');
  busy--;
  if(busy == 0) {
    proto_low_busy_lo();
    // trigger signal
    proto_trigger_signal();
  }
}

u08 proto_is_busy(void)
{
  return busy > 0;
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
  if(num > PROTO_ACTION_KNOK) {
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

// channel operations

static void handle_chn_set_rx_offset(u08 chan)
{
  u32 off = proto_low_write_long();
  proto_api_chn_set_rx_offset(chan, off);
  proto_low_end();
  DL(off); DNL;
}

static void handle_chn_set_tx_offset(u08 chan)
{
  u32 off = proto_low_write_long();
  proto_api_chn_set_tx_offset(chan, off);
  proto_low_end();
  DL(off); DNL;
}

static void handle_chn_get_rx_size(u08 chan)
{
  u16 size = proto_api_chn_get_rx_size(chan);
  proto_low_read_word(size);
  proto_low_end();
  DW(size); DNL;
}

static void handle_chn_set_rx_size(u08 chan)
{
  u16 size = proto_low_write_word();
  proto_api_chn_set_rx_size(chan, size);
  proto_low_end();
  DW(size); DNL;
}

static void handle_chn_set_tx_size(u08 chan)
{
  u16 size = proto_low_write_word();
  proto_api_chn_set_tx_size(chan, size);
  proto_low_end();
  DW(size); DNL;
}

static void handle_chn_read_data(u08 chan)
{
  // get filled buffer and send it
  u16 msg_size = 0;
  u08 *buf = proto_api_read_msg_begin(chan, &msg_size);
  if(buf == NULL) {
    DC('#');
    proto_low_read_block_spi(msg_size);
  } else {
    proto_low_read_block(msg_size, buf);
  }
  proto_api_read_msg_done(chan);

  proto_low_end();
}

static void handle_chn_write_data(u08 chan)
{
  // get buffer and fill it
  u16 msg_size = 0;
  u08 *buf = proto_api_write_msg_begin(chan, &msg_size);
  if(buf == NULL) {
    DC('#');
    proto_low_write_block_spi(msg_size);
  } else {
    proto_low_write_block(msg_size, buf);
  }
  proto_api_write_msg_done(chan);

  proto_low_end();
}

static void handle_chn_request_rx(u08 chan)
{
  proto_low_action();
  proto_api_chn_request_rx(chan);
  proto_low_end();
}

static void handle_chn_cancel_rx(u08 chan)
{
  proto_low_action();
  proto_api_chn_cancel_rx(chan);
  proto_low_end();
}

static void handle_chn_cancel_tx(u08 chan)
{
  proto_low_action();
  proto_api_chn_cancel_tx(chan);
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

    // channel commands
    case PROTO_CMD_CHN_READ_DATA:
      DC('m');
      handle_chn_read_data(chn);
      break;
    case PROTO_CMD_CHN_WRITE_DATA:
      DC('M');
      handle_chn_write_data(chn);
      break;

    case PROTO_CMD_CHN_GET_RX_SIZE:
      DC('R');
      handle_chn_get_rx_size(chn);
      break;
    case PROTO_CMD_CHN_SET_RX_SIZE:
      DC('r');
      handle_chn_set_rx_size(chn);
      break;
    case PROTO_CMD_CHN_SET_TX_SIZE:
      DC('r');
      handle_chn_set_tx_size(chn);
      break;
    case PROTO_CMD_CHN_SET_RX_OFFSET:
      DC('o');
      handle_chn_set_rx_offset(chn);
      break;
    case PROTO_CMD_CHN_SET_TX_OFFSET:
      DC('O');
      handle_chn_set_tx_offset(chn);
      break;
    case PROTO_CMD_CHN_REQUEST_RX:
      DC('x');
      handle_chn_request_rx(chn);
      break;
    case PROTO_CMD_CHN_CANCEL_RX:
      DC('c');
      handle_chn_cancel_rx(chn);
      break;
    case PROTO_CMD_CHN_CANCEL_TX:
      DC('C');
      handle_chn_cancel_tx(chn);
      break;

    default:
      DC('!'); DNL;
      // trigger a reset to re-enter knok
      system_sys_reset();
      break;
  }

  DC(']'); DB(cmd); DNL;
}

/* mini handler - reduced function set to save space */
void proto_handle_mini(void)
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
    case PROTO_CMD_LFUNC_READ:
      DC('l');
      handle_lfunc_read(chn);
      break;
    case PROTO_CMD_LFUNC_WRITE:
      DC('L');
      handle_lfunc_write(chn);
      break;
      // only read/write supported in bootloader
    case PROTO_CMD_CHN_READ_DATA:
      DC('m');
      handle_chn_read_data(chn);
      break;
    case PROTO_CMD_CHN_WRITE_DATA:
      DC('M');
      handle_chn_write_data(chn);
      break;
    default:
      DC('!'); DNL;
      // trigger a reset to re-enter knok
      system_sys_reset();
      break;
  }

  DC(']'); DB(cmd); DNL;
}
