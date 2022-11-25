#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "types.h"
#include "arch.h"

#include "pamlib.h"
#include "devices/pamela.h"

#define STATUS_OPEN     1

struct pamlib_handle {
  struct Library     *sys_base;
  struct MsgPort     *port;
  struct IOPamReq    *req;
  UWORD               status;
};

struct pamlib_channel {
  pamlib_handle_t  *handle;
  UWORD             port;
  UBYTE             channel;
};

pamlib_handle_t *pamlib_init(struct Library *SysBase, int *error, char *dev_name)
{
  // create handle
  pamlib_handle_t *ph = AllocMem(sizeof(pamlib_handle_t), MEMF_CLEAR | MEMF_PUBLIC);
  if(ph == NULL) {
    *error = PAMELA_ERROR_NO_MEM;
    return NULL;
  }

  ph->sys_base = SysBase;

  // setup message port
  ph->port = CreateMsgPort();
  if(ph->port == NULL) {
    pamlib_exit(ph);
    *error = PAMELA_ERROR_NO_MEM;
    return NULL;
  }

  // setup io request
  ph->req = (struct IOPamReq *)CreateIORequest(ph->port, sizeof(*ph->req));
  if(ph->req == NULL) {
    pamlib_exit(ph);
    *error = PAMELA_ERROR_NO_MEM;
    return NULL;
  }

  // open device
  if (OpenDevice(dev_name, 0, (struct IORequest *)ph->req, 0)) {
    pamlib_exit(ph);
    *error = PAMELA_ERROR_INIT_ENV;
    return NULL;
  }

  ph->status = STATUS_OPEN;

  return ph;
}

#undef SysBase
#define SysBase ph->sys_base

void pamlib_exit(pamlib_handle_t *ph)
{
  if(ph == NULL) {
    return;
  }

  if(ph->status != 0) {
    CloseDevice((struct IORequest *)ph->req);
  }

  if(ph->req != NULL) {
    DeleteIORequest(ph->req);
  }

  if(ph->port != NULL) {
    DeleteMsgPort(ph->port);
  }

  FreeMem(ph, sizeof(pamlib_handle_t));
}

int pamlib_devinfo(pamlib_handle_t *ph, pamela_devinfo_t *di)
{
  if(ph == NULL) {
    return PAMELA_ERROR_NO_HANDLE;
  }

  // close channel via device
  int error = 0;
  ph->req->iopam_Req.io_Command = PAMCMD_DEVINFO;
  ph->req->iopam_Req.io_Data = di;
  ph->req->iopam_Req.io_Length = sizeof(*di);
  int res = DoIO((struct IORequest *)ph->req);
  if(res != 0) {
    error = ph->req->iopam_PamelaError;
  }

  return 0;
}

UWORD pamlib_wire_error(pamlib_handle_t *ph)
{
  return ph->req->iopam_WireError;
}

pamlib_channel_t *pamlib_open(pamlib_handle_t *ph, UWORD port, int *error)
{
  // create channel handle
  pamlib_channel_t *ch = AllocMem(sizeof(pamlib_channel_t), MEMF_CLEAR | MEMF_PUBLIC);
  if(ch == NULL) {
    *error = PAMELA_ERROR_NO_MEM;
    return NULL;
  }

  // try to open channel via device
  ph->req->iopam_Req.io_Command = PAMCMD_OPEN_CHANNEL;
  ph->req->iopam_Port = port;
  int res = DoIO((struct IORequest *)ph->req);
  if(res != 0) {
    FreeMem(ch, sizeof(*ch));
    *error = ph->req->iopam_PamelaError;
    if(*error == PAMELA_ERROR_WIRE) {
      *error = ph->req->iopam_WireError;
    }
    return NULL;
  }

  ch->channel = ph->req->iopam_Channel;
  ch->handle = ph;
  ch->port = port;

  return ch;
}

int pamlib_close(pamlib_channel_t *ch)
{
  if(ch == NULL) {
    return PAMELA_ERROR_CHANNEL_NOT_FOUND;
  }

  // close channel via device
  pamlib_handle_t *ph = ch->handle;
  int error = PAMELA_OK;
  ph->req->iopam_Req.io_Command = PAMCMD_CLOSE_CHANNEL;
  ph->req->iopam_Channel = ch->channel;
  int res = DoIO((struct IORequest *)ph->req);
  if(res != 0) {
    error = ph->req->iopam_PamelaError;
  }

  FreeMem(ch, sizeof(*ch));

  return error;
}

int pamlib_reset(pamlib_channel_t *ch)
{
  if(ch == NULL) {
    return PAMELA_ERROR_CHANNEL_NOT_FOUND;
  }

  // reset channel via device
  pamlib_handle_t *ph = ch->handle;
  int error = PAMELA_OK;
  ph->req->iopam_Req.io_Command = PAMCMD_RESET_CHANNEL;
  ph->req->iopam_Channel = ch->channel;
  int res = DoIO((struct IORequest *)ph->req);
  if(res != 0) {
    error = ph->req->iopam_PamelaError;
  }
  return error;
}

int pamlib_read(pamlib_channel_t *ch, UBYTE *data, UWORD size)
{
  if(ch == NULL) {
    return PAMELA_ERROR_CHANNEL_NOT_FOUND;
  }

  // read on channel via device
  pamlib_handle_t *ph = ch->handle;
  ph->req->iopam_Req.io_Command = PAMCMD_READ;
  ph->req->iopam_Channel = ch->channel;
  ph->req->iopam_Req.io_Length = size;
  ph->req->iopam_Req.io_Data = data;
  int res = DoIO((struct IORequest *)ph->req);
  if(res != 0) {
    return ph->req->iopam_PamelaError;
  }
  return ph->req->iopam_Req.io_Actual;
}

int pamlib_write(pamlib_channel_t *ch, UBYTE *data, UWORD size)
{
  if(ch == NULL) {
    return PAMELA_ERROR_CHANNEL_NOT_FOUND;
  }

  // write to channel via device
  pamlib_handle_t *ph = ch->handle;
  ph->req->iopam_Req.io_Command = PAMCMD_WRITE;
  ph->req->iopam_Channel = ch->channel;
  ph->req->iopam_Req.io_Length = size;
  ph->req->iopam_Req.io_Data = data;
  int res = DoIO((struct IORequest *)ph->req);
  if(res != 0) {
    return ph->req->iopam_PamelaError;
  }
  return ph->req->iopam_Req.io_Actual;
}

int pamlib_seek(pamlib_channel_t *ch, ULONG offset)
{
  if(ch == NULL) {
    return PAMELA_ERROR_CHANNEL_NOT_FOUND;
  }

  // seek in channel via device
  pamlib_handle_t *ph = ch->handle;
  ph->req->iopam_Req.io_Command = PAMCMD_SEEK;
  ph->req->iopam_Channel = ch->channel;
  ph->req->iopam_Req.io_Offset = offset;
  int res = DoIO((struct IORequest *)ph->req);
  if(res != 0) {
    return ph->req->iopam_PamelaError;
  }
  return 0;
}

int pamlib_tell(pamlib_channel_t *ch, ULONG *offset)
{
  if(ch == NULL) {
    return PAMELA_ERROR_CHANNEL_NOT_FOUND;
  }

  // tell pos of channel via device
  pamlib_handle_t *ph = ch->handle;
  ph->req->iopam_Req.io_Command = PAMCMD_TELL;
  ph->req->iopam_Channel = ch->channel;
  int res = DoIO((struct IORequest *)ph->req);
  if(res != 0) {
    return ph->req->iopam_PamelaError;
  }
  *offset = ph->req->iopam_Req.io_Offset;
  return 0;
}

int pamlib_get_mtu(pamlib_channel_t *ch, UWORD *mtu)
{
  if(ch == NULL) {
    return PAMELA_ERROR_CHANNEL_NOT_FOUND;
  }

  // get mtu of channel via device
  pamlib_handle_t *ph = ch->handle;
  ph->req->iopam_Req.io_Command = PAMCMD_GET_MTU;
  ph->req->iopam_Channel = ch->channel;
  int res = DoIO((struct IORequest *)ph->req);
  if(res != 0) {
    return ph->req->iopam_PamelaError;
  }
  *mtu = (UWORD)ph->req->iopam_Req.io_Actual;
  return 0;
}

int pamlib_set_mtu(pamlib_channel_t *ch, UWORD mtu)
{
  if(ch == NULL) {
    return PAMELA_ERROR_CHANNEL_NOT_FOUND;
  }

  // set mtu of channel via device
  pamlib_handle_t *ph = ch->handle;
  ph->req->iopam_Req.io_Command = PAMCMD_SET_MTU;
  ph->req->iopam_Channel = ch->channel;
  ph->req->iopam_Req.io_Length = mtu;
  int res = DoIO((struct IORequest *)ph->req);
  if(res != 0) {
    return ph->req->iopam_PamelaError;
  }
  return 0;
}

pamlib_handle_t *pamlib_get_handle(pamlib_channel_t *pc)
{
  return pc->handle;
}

struct Library *pamlib_get_sysbase(pamlib_handle_t *ph)
{
  return ph->sys_base;
}
