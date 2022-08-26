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

#define TIMEOUT_US  500000
#define TIMEOUT_S   0

pamela_engine_t *pamela_engine_init(struct Library *SysBase, int *error)
{
  D(("pam_eng: init:"));

  // create handle
  pamela_engine_t *eng = AllocMem(sizeof(pamela_engine_t), MEMF_CLEAR);
  if(eng == NULL) {
    *error = PAMELA_ERROR_NO_MEM;
    D(("no mem!\n"));
    return NULL;
  }
  eng->quit_signal = -1;

  // setup pamela
  eng->pamela = pamela_init(SysBase, error);
  if(eng->pamela == NULL) {
    pamela_engine_exit(eng);
    D(("pam err: %ld\n", error));
    return NULL;
  }

  // keep my sys base
  eng->sys_base = SysBase;

  // client list init
  NewList((struct List *)&eng->clients);

  // alloc sockets for each channel
  eng->num_sockets = pamela_get_max_channels(eng->pamela);
  ULONG sock_size = sizeof(pamela_socket_t) * eng->num_sockets;
  eng->sockets = AllocVec(sock_size, MEMF_CLEAR);
  if(eng->sockets == NULL) {
    pamela_engine_exit(eng);
    *error = PAMELA_ERROR_NO_MEM;
    D(("no mem2!\n"));
    return NULL;
  }

  // setup msg port
  eng->req_port = CreateMsgPort();
  if(eng->req_port == NULL) {
    pamela_engine_exit(eng);
    *error = PAMELA_ERROR_NO_MEM;
    D(("no mem3!\n"));
    return NULL;
  }
  eng->port_sigmask = 1 << eng->req_port->mp_SigBit;

  // alloc quit signal
  eng->task = FindTask(NULL);
  eng->quit_signal = AllocSignal(-1);
  if(eng->quit_signal == -1) {
    pamela_engine_exit(eng);
    *error = PAMELA_ERROR_NO_MEM;
    D(("no signal!\n"));
    return NULL;
  }

  // ok
  *error = PAMELA_OK;
  D(("ok!\n"));
  return eng;
}

#undef SysBase
#define SysBase eng->sys_base

void pamela_engine_exit(pamela_engine_t *eng)
{
  D(("pam_eng: exit:"));
  if(eng == NULL) {
    return;
  }

  if(eng->pamela != NULL) {
    pamela_exit(eng->pamela);
  }
  if(eng->req_port != NULL) {
    DeleteMsgPort(eng->req_port);
  }
  if(eng->sockets != NULL) {
    FreeVec(eng->sockets);
  }
  if(eng->quit_signal != -1) {
    FreeSignal(eng->quit_signal);
  }

  FreeMem(eng, sizeof(pamela_engine_t));
  D(("done\n"));
}

static void handle_req(pamela_engine_t *eng, pamela_req_t *req)
{
  BOOL post_reply = TRUE;
  int error = PAMELA_OK;

  // clear error state
  req->iopam_PamelaError = PAMELA_OK;
  req->iopam_Req.io_Error = 0;

  D(("handle_req { cmd=%ld\n", req->iopam_Req.io_Command));
  switch(req->iopam_Req.io_Command) {
    case PAMCMD_OPEN_CHANNEL:
      error = pamela_req_open(eng, req);
      if(error == PAMELA_OK) {
        post_reply = FALSE;
      }
      break;
    case PAMCMD_CLOSE_CHANNEL:
      error = pamela_req_close(eng, req);
      if(error == PAMELA_OK) {
        post_reply = FALSE;
      }
      break;
    case PAMCMD_RESET_CHANNEL:
      error = pamela_req_reset(eng, req);
      if(error == PAMELA_OK) {
        post_reply = FALSE;
      }
      break;
    case PAMCMD_READ:
      error = pamela_req_read(eng, req);
      if(error == PAMELA_OK) {
        post_reply = FALSE;
      }
      break;
    case PAMCMD_WRITE:
      error = pamela_req_write(eng, req);
      if(error == PAMELA_OK) {
        post_reply = FALSE;
      }
      break;
    case PAMCMD_SEEK:
      error = pamela_req_seek(eng, req);
      break;
    case PAMCMD_TELL:
      error = pamela_req_tell(eng, req);
      break;
    case PAMCMD_DEVINFO:
      error = pamela_req_devinfo(eng, req);
      break;
    case PAMCMD_GET_MTU:
      error = pamela_req_get_mtu(eng, req);
      break;
    case PAMCMD_SET_MTU:
      error = pamela_req_set_mtu(eng, req);
      break;
    default:
      break;
  }
  D(("} error=%ld, post=%ld\n", error, post_reply));

  if(error != PAMELA_OK) {
    req->iopam_Req.io_Error = IOERR_PAMELA;
    req->iopam_PamelaError = error;
  }

  if(post_reply) {
    ReplyMsg((struct Message *)req);
  }
}

static void handle_timeout(pamela_engine_t *eng)
{
}

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
    req->iopam_PamelaError = PAMELA_ERROR_READ_FAILED;
  }
  else {
    // try to process buffer
    pamela_channel_t *channel = sock->channel;
    int res = pamela_read_data(channel, buf);
    if(res < 0) {
      D(("read err: %ld\n", res));
      req->iopam_Req.io_Error = IOERR_PAMELA;
      req->iopam_PamelaError = res;
    } else {
      D(("read ok: %ld\n", res));
      req->iopam_Req.io_Actual = res;
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
    req->iopam_PamelaError = PAMELA_ERROR_WRITE_FAILED;
  }
  else {
    // try to process buffer
    pamela_channel_t *channel = sock->channel;
    int res = pamela_write_data(channel, buf);
    if(res < 0) {
      D(("write err: %ld\n", res));
      req->iopam_Req.io_Error = IOERR_PAMELA;
      req->iopam_PamelaError = res;
    } else {
      D(("write ok: %ld\n", res));
      req->iopam_Req.io_Actual = res;
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
  // the read request failed
  if((status & PAMELA_STATUS_READ_ERROR) == PAMELA_STATUS_READ_ERROR) {
    handle_socket_read(eng, sock, FALSE);
  }

  // the write request is ready to be performed
  if((status & PAMELA_STATUS_WRITE_READY) == PAMELA_STATUS_WRITE_READY) {
    handle_socket_write(eng, sock, TRUE);
  }

  // the write request failed
  if((status & PAMELA_STATUS_WRITE_ERROR) == PAMELA_STATUS_WRITE_ERROR) {
    handle_socket_write(eng, sock, FALSE);
  }

  // reply command requests like open/close/reset
  UBYTE state = status & PAMELA_STATUS_STATE_MASK;
  if(state == PAMELA_STATUS_ACTIVE) {
    // open/reset cmd
    if(sock->cmd_req != NULL) {
      D(("  -> reply open/reset cmd req %lx\n", sock->cmd_req));
      ReplyMsg((struct Message *)sock->cmd_req);
      sock->cmd_req = NULL;
    }
  }
  else if(state == PAMELA_STATUS_INACTIVE) {
    // close cmd
    if(sock->cmd_req != NULL) {
      D(("  -> reply close cmd req %lx\n", sock->cmd_req));
      ReplyMsg((struct Message *)sock->cmd_req);
      sock->cmd_req = NULL;

      // cleanup socket
      pamela_engine_shutdown_socket(eng, sock);
    }
  }

  // get changed state
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

  sock->last_status = status;
}

static void handle_event(pamela_engine_t *eng)
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

ULONG pamela_engine_work(pamela_engine_t *eng, ULONG extra_sigmask)
{
  ULONG quit_sigmask = 1 << eng->quit_signal;
  while(1) {
    ULONG sigmask = extra_sigmask | eng->port_sigmask | quit_sigmask;
    // wait for pamela event, port or timeout
    D(("event wait: sigmask=%lx\n", sigmask));
    int res = pamela_event_wait(eng->pamela, TIMEOUT_S, TIMEOUT_US, &sigmask);

    // timeout?
    if((res & PAMELA_WAIT_TIMEOUT) == PAMELA_WAIT_TIMEOUT) {
      D(("-> time out!\n"));
      handle_timeout(eng);
    }

    // pamela event?
    if((res & PAMELA_WAIT_EVENT) == PAMELA_WAIT_EVENT) {
      D(("-> pam event!\n"));
      handle_event(eng);
    }

    // some signal was hit
    if((res & PAMELA_WAIT_SIGMASK) == PAMELA_WAIT_SIGMASK) {
      // an ioreq was passed in
      if((sigmask & eng->port_sigmask) == eng->port_sigmask) {
        pamela_req_t *req = (pamela_req_t *)GetMsg(eng->req_port);
        D(("-> got req: %lx\n", req));
        if(req != NULL) {
          handle_req(eng, req);
        }
      }
      // external signal hit?
      ULONG other_sigmask = sigmask & extra_sigmask;
      if(other_sigmask != 0) {
        D(("-> other sigmask: %lx\n", other_sigmask));
        // leave function and report ext sig
        return other_sigmask;
      }
      // quit?
      if((sigmask & quit_sigmask) == quit_sigmask) {
        D(("-> quit!\n"));
        return 0;
      }
    }
  }
}

void pamela_engine_quit(pamela_engine_t *eng)
{
  Signal(eng->task, 1 << eng->quit_signal);
}

int pamela_engine_init_request(pamela_engine_t *eng, pamela_req_t *req)
{
  pamela_client_t *pc = AllocMem(sizeof(pamela_client_t), MEMF_CLEAR);
  if(pc == NULL) {
    return PAMELA_ERROR_NO_MEM;
  }

  AddTail((struct List *)&eng->clients, (struct Node *)pc);

  pc->request = req;
  pc->channel_mask = 0;

  req->iopam_Internal = pc;

  return PAMELA_OK;
}

int pamela_engine_exit_request(pamela_engine_t *eng, pamela_req_t *req)
{
  pamela_client_t *pc = (pamela_client_t *)req->iopam_Internal;
  if(pc == NULL) {
    return PAMELA_ERROR_CHANNEL_NOT_FOUND;
  }

  pamela_engine_shutdown_client(eng, pc);

  Remove((struct Node *)pc->request);

  return PAMELA_OK;
}

BOOL pamela_engine_post_request(pamela_engine_t *eng, pamela_req_t *req)
{
  switch(req->iopam_Req.io_Command) {
    case PAMCMD_OPEN_CHANNEL:
    case PAMCMD_CLOSE_CHANNEL:
    case PAMCMD_RESET_CHANNEL:
    case PAMCMD_READ:
    case PAMCMD_WRITE:
    case PAMCMD_SEEK:
    case PAMCMD_TELL:
    case PAMCMD_DEVINFO:
    case PAMCMD_GET_MTU:
    case PAMCMD_SET_MTU:
      /* forward msg */
      PutMsg(eng->req_port, (struct Message *)req);
      req->iopam_Req.io_Flags &= ~IOF_QUICK;
      req->iopam_Req.io_Error = 0;
      return FALSE;
    default:
      req->iopam_Req.io_Error = IOERR_NOCMD;
      req->iopam_Req.io_Flags |= IOF_QUICK;
      return TRUE;
  }
}

void pamela_engine_shutdown_client(pamela_engine_t *eng, pamela_client_t *pc)
{
  UWORD channel_mask = pc->channel_mask;
  UWORD mask = 1;
  for(int i=0;i<eng->num_sockets;i++) {
    /* channel was opened by client/req */
    if((channel_mask & mask) == mask) {
      pamela_socket_t *sock = &eng->sockets[i];
      pamela_close(sock->channel);
      pamela_engine_shutdown_socket(eng, sock);
    }
    mask <<= 1;
  }
  pc->request = NULL;
  pc->channel_mask = 0;
}

BOOL pamela_engine_cancel_request(pamela_engine_t *eng, pamela_req_t *req)
{
  // TODO
  req->iopam_Req.io_Error = IOERR_ABORTED;
  return FALSE;
}

void pamela_engine_shutdown_socket(pamela_engine_t *eng, pamela_socket_t *sock)
{
  sock->client = NULL;
  sock->channel = NULL;

  pamela_engine_cancel_read_write(eng, sock);
}

void pamela_engine_cancel_read_write(pamela_engine_t *eng, pamela_socket_t *sock)
{
  // abort read socket
  if(sock->read_req != NULL) {
    sock->read_req->iopam_Req.io_Error = IOERR_ABORTED;
    ReplyMsg((struct Message *)sock->read_req);
    sock->read_req = NULL;
  }

  // abort write socket
  if(sock->write_req != NULL) {
    sock->write_req->iopam_Req.io_Error = IOERR_ABORTED;
    ReplyMsg((struct Message *)sock->write_req);
    sock->write_req = NULL;
  }
}

