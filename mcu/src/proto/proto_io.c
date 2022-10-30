#include "types.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_PROTO_IO

#include "arch.h"
#include "debug.h"

#include "proto_atom.h"
#include "proto_dev.h"
#include "proto_io.h"
#include "proto/wire_io.h"

static u16 event_mask;
static u08 event_irq_pending;
static u08 channel_no;

void proto_io_init()
{
  proto_dev_init();

  event_mask = 0;
  event_irq_pending = 0;
}

void proto_io_event_mask_add_chn(u08 chn)
{
  event_mask |= 1 << chn;
  DC('{'); DW(event_mask);

  // trigger an irq
  if(!event_irq_pending) {
    proto_atom_pulse_irq();
    event_irq_pending = 1;
    DC('!');
  }

  DC('}');
}

// ----- global cmd -----

static void handle_global_cmd(u08 cmd)
{
  switch(cmd) {
    case PROTO_IO_CMD_RWORD_EVENT_MASK:
      proto_atom_read_word(event_mask);
      // clear on read
      DS("[evmask:"); DW(event_mask); DC(']'); DNL;
      event_mask = 0;
      event_irq_pending = 0;
      break;
    case PROTO_IO_CMD_RWORD_DEFAULT_MTU:
      {
        u16 mtu = proto_io_api_get_default_mtu();
        proto_atom_read_word(mtu);
        break;
      }
    case PROTO_IO_CMD_RWORD_MAX_CHANNELS:
      {
        u16 max_chn = proto_io_api_get_max_channels();
        proto_atom_read_word(max_chn);
        break;
      }
    case PROTO_IO_CMD_WWORD_CHANNEL_NO:
      {
        u16 no = proto_atom_write_word();
        channel_no = (u08)(no & 0xff);
        break;
      }
    case PROTO_IO_CMD_RWORD_CHANNEL_MTU:
      {
        u16 mtu = proto_io_api_get_channel_mtu(channel_no);
        proto_atom_read_word(mtu);
        break;
      }
    case PROTO_IO_CMD_WWORD_CHANNEL_MTU:
      {
        u16 mtu = proto_atom_write_word();
        proto_io_api_set_channel_mtu(channel_no, mtu);
        break;
      }
    default:
      DC('!'); DC('C'); DB(cmd); DNL;
      break;
  }
}

// ----- channel control -----

static void handle_open(u08 chn)
{
  u16 port = proto_atom_write_word();
  proto_io_api_open(chn, port);
}

static void handle_close(u08 chn)
{
  proto_atom_action();
  proto_io_api_close(chn);
}

static void handle_status(u08 chn)
{
  u16 status = proto_io_api_status(chn);
  proto_atom_read_word(status);
}

static void handle_reset(u08 chn)
{
  proto_atom_action();
  proto_io_api_reset(chn);
}

static void handle_seek(u08 chn)
{
  u32 off = proto_atom_write_long();
  proto_io_api_seek(chn, off);
}

static void handle_tell(u08 chn)
{
  u32 off = proto_io_api_tell(chn);
  proto_atom_read_long(off);
}

// ----- read -----

static void handle_read_req(u08 chn)
{
  u16 size = proto_atom_write_word();
  proto_io_api_read_req(chn, size);
}

static void handle_read_res(u08 chn)
{
  u16 size = proto_io_api_read_res(chn);
  proto_atom_read_word(size);
}

static void handle_read_blk(u08 chn)
{
  u16 size = 0;
  u08 *buf = NULL;
  proto_io_api_read_blk(chn, &size, &buf);

  /* odd size handling: read next even block.
     buf should be always large enough to keep the even size!
  */
  u16 transfer_size = size;
  if((transfer_size & 1) != 0) {
    transfer_size++;
  }
  proto_atom_read_block(buf, transfer_size);

  proto_io_api_read_done(chn, size, buf);
}

// ----- write -----

static void handle_write_req(u08 chn)
{
  u16 size = proto_atom_write_word();
  proto_io_api_write_req(chn, size);
}

static void handle_write_res(u08 chn)
{
  u16 size = proto_io_api_write_res(chn);
  proto_atom_read_word(size);
}

static void handle_write_blk(u08 chn)
{
  u16 size = 0;
  u08 *buf = NULL;
  proto_io_api_write_blk(chn, &size, &buf);

  /* odd size handling: read next even block.
     buf should be always large enough to keep the even size!
  */
  u16 transfer_size = size;
  if((transfer_size & 1) != 0) {
    transfer_size++;
  }
  proto_atom_write_block(buf, size);

  proto_io_api_write_done(chn, size, buf);
}

// ----- cmd dispatch -----

void proto_io_handle_cmd()
{
  // get command/channel and split it
  u08 cmd_chn = proto_dev_get_cmd();
  if(cmd_chn == PROTO_NO_CMD) {
    return;
  }

  u08 cmd = cmd_chn & PROTO_IO_CMD_MASK;
  u08 chn = cmd_chn & PROTO_IO_CHANNEL_MASK;
  switch(cmd) {
    // global config range
    case PROTO_IO_CMD_GLOBAL_MASK:
      handle_global_cmd(cmd_chn);
      break;
    // channel control
    case PROTO_IO_CMD_WWORD_OPEN:
      handle_open(chn);
      break;
    case PROTO_IO_CMD_ACTION_CLOSE:
      handle_close(chn);
      break;
    case PROTO_IO_CMD_RWORD_STATUS:
      handle_status(chn);
      break;
    case PROTO_IO_CMD_ACTION_RESET:
      handle_reset(chn);
      break;
    case PROTO_IO_CMD_WLONG_SEEK:
      handle_seek(chn);
      break;
    case PROTO_IO_CMD_RLONG_TELL:
      handle_tell(chn);
      break;
    // read
    case PROTO_IO_CMD_WWORD_READ_REQ:
      handle_read_req(chn);
      break;
    case PROTO_IO_CMD_RWORD_READ_RESULT:
      handle_read_res(chn);
      break;
    case PROTO_IO_CMD_RBLOCK_READ_DATA:
      handle_read_blk(chn);
      break;
    // write
    case PROTO_IO_CMD_WWORD_WRITE_REQ:
      handle_write_req(chn);
      break;
    case PROTO_IO_CMD_RWORD_WRITE_RESULT:
      handle_write_res(chn);
      break;
    case PROTO_IO_CMD_WBLOCK_WRITE_DATA:
      handle_write_blk(chn);
      break;
    default:
      DC('!'); DC('O'); DB(cmd); DNL;
      break;
  }
}
