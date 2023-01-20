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
#include "pamela_sock.h"

#undef SysBase
#define SysBase eng->sys_base

static void handle_socket_read(pamela_engine_t *eng, pamela_socket_t *sock, BOOL ok)
{
  D(("sock read: ok=%ld\n", ok));

  // make sure a read req is associated
  pamela_req_t *req = sock->read_req;
  if(req == NULL) {
    D(("no read req??\n"));
    return;
  }

  // buffer
  APTR buf = req->iopam_Req.io_Data;
  if(buf == NULL) {
    D(("no data??\n"));
    ok = FALSE;
  }

  // already error?
  if(!ok) {
    req->iopam_Req.io_Error = IOERR_PAMELA;
    req->iopam_PamelaError = PAMELA_ERROR_WIRE;
    req->iopam_WireError = pamela_error(sock->channel);
  }
  else {
    // try to process buffer
    pamela_channel_t *channel = sock->channel;
    int res = pamela_read_setup(channel);
    int sum = 0;
    if(res == PAMELA_OK) {
      while(1) {
        res = pamela_read_block(channel);
        if(res <= 0) {
          break;
        }
        sum += res;
      }
    }
    if(res < 0) {
      D(("read err: %ld\n", res));
      req->iopam_Req.io_Error = IOERR_PAMELA;
      req->iopam_PamelaError = res;
    } else {
      D(("read ok: %ld\n", res));
      req->iopam_Req.io_Actual = sum;
      req->iopam_Req.io_Error = 0;
    }
  }

  ReplyMsg((struct Message *)req);

  sock->read_req = NULL;
}

static void handle_socket_write(pamela_engine_t *eng, pamela_socket_t *sock, BOOL ok)
{
  D(("sock write: ok=%ld\n", ok));

  // make sure a write req is associated
  pamela_req_t *req = sock->write_req;
  if(req == NULL) {
    D(("no read req??\n"));
    return;
  }

  // buffer
  APTR buf = req->iopam_Req.io_Data;
  if(buf == NULL) {
    D(("no data??\n"));
    ok = FALSE;
  }

  // already error?
  if(!ok) {
    req->iopam_Req.io_Error = IOERR_PAMELA;
    req->iopam_PamelaError = PAMELA_ERROR_WIRE;
    req->iopam_WireError = pamela_error(sock->channel);
  }
  else {
    // try to process buffer
    pamela_channel_t *channel = sock->channel;
    int res = pamela_write_setup(channel);
    int sum = 0;
    if(res == PAMELA_OK) {
      while(1) {
        res = pamela_write_block(channel);
        if(res <= 0) {
          break;
        }
        sum += res;
      }
    }
    if(res < 0) {
      D(("write err: %ld\n", res));
      req->iopam_Req.io_Error = IOERR_PAMELA;
      req->iopam_PamelaError = res;
    } else {
      D(("write ok: %ld\n", res));
      req->iopam_Req.io_Actual = sum;
      req->iopam_Req.io_Error = 0;
    }
  }

  ReplyMsg((struct Message *)req);

  sock->write_req = NULL;
}

static void handle_socket_eos(pamela_engine_t *eng, pamela_socket_t *sock)
{
  D(("sock eos\n"));
}

static void handle_socket_error(pamela_engine_t *eng, pamela_socket_t *sock)
{
  D(("sock error\n"));

  // pending read request?
  if(sock->read_req != NULL) {
    handle_socket_read(eng, sock, FALSE);
  }
  // pending write request?
  if(sock->write_req != NULL) {
    handle_socket_write(eng, sock, FALSE);
  }
}

static void set_wire_error(pamela_socket_t *sock, UWORD wire_error)
{
  pamela_req_t *req = sock->cmd_req;
  if(req != NULL) {
    D(("set cmd req=%lx wire error %ld\n", req, (ULONG)wire_error));
    req->iopam_Req.io_Error = IOERR_PAMELA;
    req->iopam_PamelaError = PAMELA_ERROR_WIRE;
    req->iopam_WireError = wire_error;
  }
}

static void reply_cmd_req(pamela_engine_t *eng, pamela_socket_t *sock)
{
  pamela_req_t *req = sock->cmd_req;
  if(req != NULL) {
    D(("  -> reply cmd req=%lx\n", req));
    ReplyMsg((struct Message *)req);
    sock->cmd_req = NULL;
  }
}

static void handle_socket_status(pamela_engine_t *eng, pamela_socket_t *sock)
{
  pamela_channel_t *channel = sock->channel;

  UWORD status = pamela_status(channel);
  D(("sock #%ld: status=%04lx\n", pamela_channel_id(channel), status));

  // the read request is ready to be performed
  if((status & PAMELA_STATUS_READ_READY) == PAMELA_STATUS_READ_READY) {
    handle_socket_read(eng, sock, TRUE);
  }

  // the write request is ready to be performed
  if((status & PAMELA_STATUS_WRITE_READY) == PAMELA_STATUS_WRITE_READY) {
    handle_socket_write(eng, sock, TRUE);
  }

  // get changed state
  UBYTE state = status & PAMELA_STATUS_STATE_MASK;
  UBYTE old_state = sock->last_status & PAMELA_STATUS_STATE_MASK;
  if(state != old_state) {
    D((" -> new_state=%ld\n", state));
    switch(state) {
      case PAMELA_STATUS_EOS:
        handle_socket_eos(eng, sock);
        break;
      case PAMELA_STATUS_ERROR:
        handle_socket_error(eng, sock);
        break;
      default:
        break;
    }
  }

  // reply command requests like open/close/reset
  if(state == PAMELA_STATUS_ACTIVE) {
    D((" -> ACTIVE\n"));

    // get mtu - for read/write to work
    UWORD mtu = 0;
    int res =pamela_get_mtu(channel, &mtu);
    if(res != PAMELA_OK) {
      D(("  MTU failed!"));
      set_wire_error(sock, res);
      pamela_close(sock->channel);
    }

    reply_cmd_req(eng, sock);
  }
  else if(state == PAMELA_STATUS_INACTIVE) {
    D((" -> INACTIVE\n"));
    // cleanup socket
    pamela_sock_shutdown_socket(eng, sock);
    // answer req
    reply_cmd_req(eng, sock);
  }
  else if(state == PAMELA_STATUS_OPEN_ERROR) {
    D((" -> OPEN ERROR\n"));
    // set wire error in cmd_req
    set_wire_error(sock, pamela_error(sock->channel));
    // close channel... cmd req reply and cleanup will be done in inactive state
    pamela_close(sock->channel);
  }

  sock->last_status = status;
}

void pamela_sock_event(pamela_engine_t *eng)
{
  UWORD event_mask = 0;
  pamela_event_update(eng->pamela, &event_mask);

  // run through sockets
  UWORD sock_mask = 1;
  for(int i=0;i<eng->num_sockets;i++) {
    if((event_mask & sock_mask) == sock_mask) {
      pamela_socket_t *sock = &eng->sockets[i];
      handle_socket_status(eng, sock);
    }
    sock_mask <<= 1;
  }
}

BOOL pamela_sock_work(pamela_engine_t *eng)
{
  return TRUE;
}

void pamela_sock_timeout(pamela_engine_t *eng)
{
}

void pamela_sock_shutdown_client(pamela_engine_t *eng, pamela_client_t *pc)
{
  UWORD channel_mask = pc->channel_mask;
  UWORD mask = 1;
  for(int i=0;i<eng->num_sockets;i++) {
    /* channel was opened by client/req */
    if((channel_mask & mask) == mask) {
      pamela_socket_t *sock = &eng->sockets[i];
      D(("shutdown_client: sock=%lx\n", sock));
      pamela_close(sock->channel);
      pamela_sock_shutdown_socket(eng, sock);
    }
    mask <<= 1;
  }
  pc->request = NULL;
  pc->channel_mask = 0;
}

void pamela_sock_shutdown_socket(pamela_engine_t *eng, pamela_socket_t *sock)
{
  D(("shutdown_socket: sock=%lx\n", sock));

  // remove channel from mask
  sock->client->channel_mask &= ~(1 << pamela_channel_id(sock->channel));

  sock->client = NULL;
  sock->channel = NULL;

  pamela_sock_cancel_read_write(eng, sock);
}

void pamela_sock_cancel_read_write(pamela_engine_t *eng, pamela_socket_t *sock)
{
  // abort read socket
  if(sock->read_req != NULL) {
    D(("cancel_read: req=%lx\n", sock->read_req));
    sock->read_req->iopam_Req.io_Error = IOERR_ABORTED;
    ReplyMsg((struct Message *)sock->read_req);
    sock->read_req = NULL;
  }

  // abort write socket
  if(sock->write_req != NULL) {
    D(("cancel_write: req=%lx\n", sock->write_req));
    sock->write_req->iopam_Req.io_Error = IOERR_ABORTED;
    ReplyMsg((struct Message *)sock->write_req);
    sock->write_req = NULL;
  }
}

