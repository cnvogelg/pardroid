#include <proto/exec.h>

#include "proto_atom.h"
#include "proto_io.h"
#include "pamela.h"
#include "pamela_int.h"

void pamela_channels_init(pamela_handle_t *ph)
{
  // prepare channels
  for(int i=0;i<PROTO_IO_NUM_CHANNELS;i++) {
    pamela_channel_t *ch = &ph->channels[i];
    ch->channel_id = i;
    ch->flags = CHANNEL_FLAG_INACTIVE;
    ch->pamela = ph;
  }
}

static pamela_channel_t *find_free_channel(pamela_handle_t *ph)
{
  for(int i=0;i<PROTO_IO_NUM_CHANNELS;i++) {
    pamela_channel_t *ch = &ph->channels[i];
    if(ch->flags == CHANNEL_FLAG_INACTIVE) {
      return ch;
    }
  }
  return NULL;
}

/* open channel to given service */
pamela_channel_t *pamela_open(pamela_handle_t *ph, UWORD port, int *error)
{
  // find free channel
  pamela_channel_t *ch = find_free_channel(ph);
  if(ch == NULL) {
    *error = PAMELA_ERROR_NO_FREE_CHANNEL;
    return NULL;
  }

  // try to open channel on device
  int res = proto_io_open(ph->proto, ch->channel_id, port);
  if(res != PROTO_RET_OK) {
    *error = pamela_map_proto_error(res);
    return NULL;
  }

  // update state
  ch->port = port;

  return ch;
}

/* close channel */
int pamela_close(pamela_channel_t *ch)
{
  pamela_handle_t *ph = ch->pamela;

  // close channel on device
  int res = proto_io_close(ph->proto, ch->channel_id);
  if(res != PROTO_RET_OK) {
    return pamela_map_proto_error(res);
  }

  // update state
  ch->port = 0;

  return PAMELA_OK;
}

/* reset channel */
int pamela_reset(pamela_channel_t *ch)
{
  pamela_handle_t *ph = ch->pamela;

  /* you can only reset an open channel */
  if(ch->flags != CHANNEL_FLAG_ACTIVE) {
    return PAMELA_ERROR_CHANNEL_NOT_ACTIVE;
  }

  // reset channel on device
  int res = proto_io_reset(ph->proto, ch->channel_id);
  if(res != PROTO_RET_OK) {
    return pamela_map_proto_error(res);
  }

  // update status
  return PAMELA_OK;
}

/* get current status value */
int pamela_update(pamela_channel_t *ch)
{
  int res = proto_io_status(ch->pamela->proto, ch->channel_id, &ch->status);
  if(res != PROTO_RET_OK) {
    return pamela_map_proto_error(res);
  }

  // update internal flag
  UBYTE state = ch->status & PAMELA_STATUS_STATE_MASK;
  UBYTE flag = 0;
  switch(state) {
    case PAMELA_STATUS_INACTIVE:
      flag = CHANNEL_FLAG_INACTIVE;
      break;
    case PAMELA_STATUS_ACTIVE:
      flag = CHANNEL_FLAG_ACTIVE;
      break;
    default:
      flag = CHANNEL_FLAG_BUSY;
      break;
  }
  ch->flags = flag;

  return PAMELA_OK;
}

UWORD pamela_status(pamela_channel_t *ch)
{
  return ch->status;
}

UBYTE pamela_channel_id(pamela_channel_t *ch)
{
  return ch->channel_id;
}

static int check_channel_status(pamela_channel_t *ch, UWORD set_mask, UWORD clr_mask)
{
  UWORD status = ch->status;

  // make sure channel is active
  UBYTE state = status & PAMELA_STATUS_STATE_MASK;
  if(state != PAMELA_STATUS_ACTIVE) {
    return PAMELA_ERROR_CHANNEL_NOT_ACTIVE;
  }

  // set mask?
  if(set_mask != 0) {
    if((status & set_mask) != set_mask) {
      return PAMELA_ERROR_CHANNEL_STATE;
    }
  }

  // clk mask?
  if(clr_mask != 0) {
    if((status & clr_mask) != 0) {
      return PAMELA_ERROR_CHANNEL_STATE;
    }
  }

  return PAMELA_OK;
}

int pamela_get_mtu(pamela_channel_t *ch, UWORD *mtu)
{
  // make sure channel is active
  int res = check_channel_status(ch, 0, 0);
  if(res != PAMELA_OK) {
    return res;
  }

  // read back effective value
  res = proto_io_get_channel_mtu(ch->pamela->proto, ch->channel_id, mtu);
  if(res != PROTO_RET_OK) {
    return pamela_map_proto_error(res);
  }

  return PAMELA_OK;
}

int pamela_set_mtu(pamela_channel_t *ch, UWORD mtu)
{
  // make sure channel is active
  int res = check_channel_status(ch, 0, 0);
  if(res != PAMELA_OK) {
    return res;
  }

  // try to set the new mtu
  res = proto_io_set_channel_mtu(ch->pamela->proto, ch->channel_id, mtu);
  if(res != PROTO_RET_OK) {
    return pamela_map_proto_error(res);
  }

  // read back effective value
  UWORD got_mtu;
  res = proto_io_get_channel_mtu(ch->pamela->proto, ch->channel_id, &got_mtu);
  if(res != PROTO_RET_OK) {
    return pamela_map_proto_error(res);
  }

  if(mtu != got_mtu) {
    return PAMELA_ERROR_INVALID_MTU;
  } else {
    return PAMELA_OK;
  }
}

int pamela_seek(pamela_channel_t *ch, ULONG pos)
{
  // make sure channel is active
  int res = check_channel_status(ch, 0, 0);
  if(res != PAMELA_OK) {
    return res;
  }

  res = proto_io_seek(ch->pamela->proto, ch->channel_id, pos);
  if(res != PROTO_RET_OK) {
    return pamela_map_proto_error(res);
  }

  return PAMELA_OK;
}

int pamela_tell(pamela_channel_t *ch, ULONG *pos)
{
  // make sure channel is active
  int res = check_channel_status(ch, 0, 0);
  if(res != PAMELA_OK) {
    return res;
  }

  res = proto_io_tell(ch->pamela->proto, ch->channel_id, pos);
  if(res != PROTO_RET_OK) {
    return pamela_map_proto_error(res);
  }

  return PAMELA_OK;
}

int pamela_read_request(pamela_channel_t *ch, UWORD size)
{
  // first make sure no request is pending
  int res = check_channel_status(ch, 0, PAMELA_STATUS_READ_REQ);
  if(res != PAMELA_OK) {
    return res;
  }

  // odd size?
  if((size & 1) != 0) {
    return PAMELA_ERROR_PROTO_ODD_SIZE;
  }

  // send request to device
  res = proto_io_read_request(ch->pamela->proto, ch->channel_id, size);
  if(res != PROTO_RET_OK) {
    return pamela_map_proto_error(res);
  }

  // update my state
  ch->status |= PAMELA_STATUS_READ_REQ;
  ch->read_bytes = size;
  return PAMELA_OK;
}

int pamela_read_data(pamela_channel_t *ch, UBYTE *buf)
{
  // first make sure a request is ready
  int res = check_channel_status(ch, PAMELA_STATUS_READ_READY, 0);
  if(res != PAMELA_OK) {
    return res;
  }

  proto_handle_t *proto = ch->pamela->proto;

  // do new need to retreive an updated size?
  UWORD size = ch->read_bytes;
  if((ch->status & PAMELA_STATUS_READ_SIZE) != 0) {
    res = proto_io_read_result(proto, ch->channel_id, &size);
    if(res != PROTO_RET_OK) {
      return pamela_map_proto_error(res);
    }
  }

  // retrieve data
  res = proto_io_read_data(proto, ch->channel_id, buf, size);
  if(res != PROTO_RET_OK) {
    return pamela_map_proto_error(res);
  }

  // update state
  ch->status &= ~PAMELA_STATUS_READ_MASK;
  ch->read_bytes = 0;

  return size;
}

int pamela_write_request(pamela_channel_t *ch, UWORD size)
{
  // first make sure no request is pending
  int res = check_channel_status(ch, 0, PAMELA_STATUS_WRITE_REQ);
  if(res != PAMELA_OK) {
    return res;
  }

  // odd size?
  if((size & 1) != 0) {
    return PAMELA_ERROR_PROTO_ODD_SIZE;
  }

  // send request to device
  res = proto_io_write_request(ch->pamela->proto, ch->channel_id, size);
  if(res != PROTO_RET_OK) {
    return pamela_map_proto_error(res);
  }

  // update my state
  ch->status |= PAMELA_STATUS_WRITE_REQ;
  ch->write_bytes = size;
  return PAMELA_OK;
}

int pamela_write_data(pamela_channel_t *ch, UBYTE *buf)
{
  // first make sure a request is ready
  int res = check_channel_status(ch, PAMELA_STATUS_WRITE_READY, 0);
  if(res != PAMELA_OK) {
    return res;
  }

  proto_handle_t *proto = ch->pamela->proto;

  // do new need to retreive an updated size?
  UWORD size = ch->write_bytes;
  if((ch->status & PAMELA_STATUS_WRITE_SIZE) != 0) {
    res = proto_io_write_result(proto, ch->channel_id, &size);
    if(res != PROTO_RET_OK) {
      return pamela_map_proto_error(res);
    }
  }

  // write data
  res = proto_io_write_data(proto, ch->channel_id, buf, size);
  if(res != PROTO_RET_OK) {
    return pamela_map_proto_error(res);
  }

  // update state
  ch->status &= ~PAMELA_STATUS_WRITE_MASK;
  ch->write_bytes = 0;

  return size;
}
