#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pario.h"
#include "timer.h"
#include "proto_atom.h"
#include "proto_dev.h"
#include "proto_io.h"
#include "proto_io_shared.h"

proto_handle_t *proto_io_init(proto_env_handle_t *penv)
{
  proto_handle_t *ph = proto_dev_init(penv);
  if(ph == NULL) {
    return NULL;
  }
  return ph;
}

void proto_io_exit(proto_handle_t *ph)
{
  proto_dev_exit(ph);
}

int proto_io_get_event_mask(proto_handle_t *ph, UWORD *events)
{
  return proto_atom_read_word(ph, PROTO_IO_CMD_RWORD_EVENT_MASK, events);
}

int proto_io_get_default_mtu(proto_handle_t *ph, UWORD *mtu)
{
  return proto_atom_read_word(ph, PROTO_IO_CMD_RWORD_DEFAULT_MTU, mtu);
}

int proto_io_get_max_channels(proto_handle_t *ph, UWORD *channels)
{
  return proto_atom_read_word(ph, PROTO_IO_CMD_RWORD_MAX_CHANNELS, channels);
}

int proto_io_get_channel_mtu(proto_handle_t *ph, channel_t ch, UWORD *mtu)
{
  // set current channel
  int res = proto_atom_write_word(ph, PROTO_IO_CMD_WWORD_CHANNEL_NO, ch);
  if(res != PROTO_RET_OK) {
    return res;
  }
  return proto_atom_read_word(ph, PROTO_IO_CMD_RWORD_CHANNEL_MTU, mtu);
}

int proto_io_set_channel_mtu(proto_handle_t *ph, channel_t ch, UWORD mtu)
{
  // set current channel
  int res = proto_atom_write_word(ph, PROTO_IO_CMD_WWORD_CHANNEL_NO, ch);
  if(res != PROTO_RET_OK) {
    return res;
  }
  return proto_atom_write_word(ph, PROTO_IO_CMD_WWORD_CHANNEL_MTU, mtu);
}

/* channel commands */
int proto_io_open(proto_handle_t *ph, channel_t ch, UWORD port)
{
  UBYTE cmd = PROTO_IO_CMD_WWORD_OPEN + ch;
  return proto_atom_write_word(ph, cmd, port);
}

int proto_io_close(proto_handle_t *ph, channel_t ch)
{
  UBYTE cmd = PROTO_IO_CMD_ACTION_CLOSE + ch;
  return proto_atom_action(ph, cmd);
}

int proto_io_status(proto_handle_t *ph, channel_t ch, UWORD *status)
{
  UBYTE cmd = PROTO_IO_CMD_RWORD_STATUS + ch;
  return proto_atom_read_word(ph, cmd, status);
}

int proto_io_reset(proto_handle_t *ph, channel_t ch)
{
  UBYTE cmd = PROTO_IO_CMD_ACTION_RESET + ch;
  return proto_atom_action(ph, cmd);
}

int proto_io_seek(proto_handle_t *ph, channel_t ch, offset_t offset)
{
  UBYTE cmd = PROTO_IO_CMD_WLONG_SEEK + ch;
  return proto_atom_write_long(ph, cmd, (ULONG)offset);
}

int proto_io_tell(proto_handle_t *ph, channel_t ch, offset_t *offset)
{
  UBYTE cmd = PROTO_IO_CMD_RLONG_TELL + ch;
  return proto_atom_read_long(ph, cmd, (ULONG *)offset);
}

/* read */
int proto_io_read_request(proto_handle_t *ph, channel_t ch, io_size_t size)
{
  UBYTE cmd = PROTO_IO_CMD_WWORD_READ_REQ + ch;
  return proto_atom_write_word(ph, cmd, (UWORD)size);
}

int proto_io_read_result(proto_handle_t *ph, channel_t ch, io_size_t *size)
{
  UBYTE cmd = PROTO_IO_CMD_RWORD_READ_RESULT + ch;
  return proto_atom_read_word(ph, cmd, (UWORD *)size);
}

int proto_io_read_data(proto_handle_t *ph, channel_t ch, UBYTE *data, io_size_t size)
{
  UBYTE cmd = PROTO_IO_CMD_RBLOCK_READ_DATA + ch;
  return proto_atom_read_block(ph, cmd, data, size);
}

/* write */
int proto_io_write_request(proto_handle_t *ph, channel_t ch, io_size_t size)
{
  UBYTE cmd = PROTO_IO_CMD_WWORD_WRITE_REQ + ch;
  return proto_atom_write_word(ph, cmd, (UWORD)size);
}

int proto_io_write_result(proto_handle_t *ph, channel_t ch, io_size_t *size)
{
  UBYTE cmd = PROTO_IO_CMD_RWORD_WRITE_RESULT + ch;
  return proto_atom_read_word(ph, cmd, (UWORD *)size);
}

int proto_io_write_data(proto_handle_t *ph, channel_t ch, UBYTE *data, io_size_t size)
{
  UBYTE cmd = PROTO_IO_CMD_WBLOCK_WRITE_DATA + ch;
  return proto_atom_write_block(ph, cmd, data, size);
}
