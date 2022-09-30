#define __NOLIBBASE__
#include <proto/exec.h>
#include <clib/alib_protos.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_PAMELA_ENGINE
#define KDEBUG
#endif

#include "compiler.h"
#include "debug.h"

#include "types.h"
#include "arch.h"

#include "pamela.h"
#include "pamela_engine.h"
#include "pamela_engine_int.h"
#include "pamela_req.h"

static pamela_socket_t *get_socket(pamela_engine_t *eng, pamela_req_t *req)
{
  UBYTE channel = pamela_req_get_channel(req);
  if(channel >= eng->num_sockets) {
    req->iopam_Req.io_Error = IOERR_BADLENGTH;
    return NULL;
  }
  return &eng->sockets[channel];
}

int pamela_req_open(pamela_engine_t *eng, pamela_req_t *req)
{
  // open channel in pamela
  UWORD port = req->iopam_Port;
  int error = 0;
  pamela_channel_t *ch = pamela_open(eng->pamela, port, &error);
  if(ch == NULL) {
    return error;
  }

  // get and store channel id
  UBYTE channel_id = pamela_channel_id(ch);
  req->iopam_Channel = channel_id;

  // setup socket
  pamela_client_t *client = pamela_req_get_client(req);
  pamela_socket_t *sock = &eng->sockets[channel_id];
  sock->channel = ch;
  sock->client = client;
  sock->read_req = NULL;
  sock->write_req = NULL;
  sock->cmd_req = req;
  sock->last_status = PAMELA_STATUS_INACTIVE;

  // update channel client mask
  client->channel_mask |= 1 << channel_id;

  D(("[OPEN]\n"));
  return PAMELA_OK;
}

int pamela_req_close(pamela_engine_t *eng, pamela_req_t *req)
{
  pamela_client_t *client = pamela_req_get_client(req);
  pamela_socket_t *socket = get_socket(eng, req);
  if(socket == NULL) {
    D(("CLOSE: no socket?\n"));
    return PAMELA_OK; // error already stored
  }

  // close pamela channel
  pamela_channel_t *channel = socket->channel;
  int res = pamela_close(channel);
  if(res != PAMELA_OK) {
    D(("CLOSE: pam close failed?\n"));
    return res;
  }

  socket->cmd_req = req;

  // update channel mask
  client->channel_mask &= ~(1 << req->iopam_Channel);

  D(("[CLOSE]\n"));
  return PAMELA_OK;
}

int pamela_req_reset(pamela_engine_t *eng, pamela_req_t *req)
{
  pamela_socket_t *socket = get_socket(eng, req);
  if(socket == NULL) {
    return PAMELA_OK; // error already stored
  }

  // reset pamela channel
  pamela_channel_t *channel = socket->channel;
  int res = pamela_reset(channel);
  if(res != PAMELA_OK) {
    return res;
  }

  socket->cmd_req = req;

  // cancel all pending requests at socket
  pamela_engine_cancel_read_write(eng, socket);

  return PAMELA_OK;
}

int pamela_req_seek(pamela_engine_t *eng, pamela_req_t *req)
{
  pamela_socket_t *socket = get_socket(eng, req);
  if(socket == NULL) {
    return PAMELA_OK; // error already stored
  }

  ULONG offset = req->iopam_Req.io_Offset;

  // seek
  pamela_channel_t *channel = socket->channel;
  int res = pamela_seek(channel, offset);
  if(res != PAMELA_OK) {
    return res;
  }

  return PAMELA_OK;
}

int pamela_req_tell(pamela_engine_t *eng, pamela_req_t *req)
{
  pamela_socket_t *socket = get_socket(eng, req);
  if(socket == NULL) {
    return PAMELA_OK; // error already stored
  }

  // tell
  pamela_channel_t *channel = socket->channel;
  ULONG offset = 0;
  int res = pamela_tell(channel, &offset);
  if(res != PAMELA_OK) {
    return res;
  }

  req->iopam_Req.io_Offset = offset;

  return PAMELA_OK;
}

int pamela_req_read(pamela_engine_t *eng, pamela_req_t *req)
{
  pamela_socket_t *socket = get_socket(eng, req);
  if(socket == NULL) {
    return PAMELA_OK; // error already stored
  }

  // is a request already pending?
  if(socket->read_req != NULL) {
    return PAMELA_ERROR_ALREADY_READING;
  }

  UWORD size = (UWORD)req->iopam_Req.io_Length;
  D(("req_read: size=%ld\n", size));

  // start read
  pamela_channel_t *channel = socket->channel;
  int res = pamela_read_request(channel, size);
  if(res != PAMELA_OK) {
    D(("-> res=%ld\n", res));
    return res;
  }

  // keep for later
  socket->read_req = req;

  return PAMELA_OK;
}

int pamela_req_write(pamela_engine_t *eng, pamela_req_t *req)
{
  pamela_socket_t *socket = get_socket(eng, req);
  if(socket == NULL) {
    return PAMELA_OK; // error already stored
  }

  // is a request already pending?
  if(socket->write_req != NULL) {
    return PAMELA_ERROR_ALREADY_WRITING;
  }

  UWORD size = (UWORD)req->iopam_Req.io_Length;

  // start write
  pamela_channel_t *channel = socket->channel;
  int res = pamela_write_request(channel, size);
  if(res != PAMELA_OK) {
    return res;
  }

  // keep for later
  socket->write_req = req;

  return PAMELA_OK;
}

int pamela_req_devinfo(pamela_engine_t *eng, pamela_req_t *req)
{
  APTR data = req->iopam_Req.io_Data;
  ULONG length = req->iopam_Req.io_Length;
  // check size and copy dev info
  if(length >= sizeof(pamela_devinfo_t)) {
    pamela_devinfo_t *ptr = (pamela_devinfo_t *)data;
    pamela_devinfo(eng->pamela, ptr);
  } else {
    req->iopam_Req.io_Error = IOERR_BADLENGTH;
  }
  D(("[DEVINFO]\n"))
  return PAMELA_OK;
}

int pamela_req_get_mtu(pamela_engine_t *eng, pamela_req_t *req)
{
  pamela_socket_t *socket = get_socket(eng, req);
  if(socket == NULL) {
    return PAMELA_OK; // error already stored
  }

  UWORD mtu = 0;
  pamela_channel_t *channel = socket->channel;
  int res = pamela_get_mtu(channel, &mtu);
  D(("[GET_MTU=%ld:%ld]\n",(LONG)mtu,(LONG)res));
  if(res != PAMELA_OK) {
    req->iopam_Req.io_Actual = 0;
    return res;
  }
  req->iopam_Req.io_Actual = mtu;
  return PAMELA_OK;
}

int pamela_req_set_mtu(pamela_engine_t *eng, pamela_req_t *req)
{
  pamela_socket_t *socket = get_socket(eng, req);
  if(socket == NULL) {
    return PAMELA_OK; // error already stored
  }

  UWORD mtu = (UWORD)req->iopam_Req.io_Length;
  pamela_channel_t *channel = socket->channel;
  int res = pamela_set_mtu(channel, mtu);
  D(("[SET_MTU=%ld:%ld]\n",(LONG)mtu,(LONG)res));
  if(res != PAMELA_OK) {
    return res;
  }
  return PAMELA_OK;
}
