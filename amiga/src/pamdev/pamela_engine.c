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
#include "pamela_sock.h"

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

ULONG pamela_engine_work(pamela_engine_t *eng, ULONG extra_sigmask)
{
  ULONG quit_sigmask = 1 << eng->quit_signal;
  while(1) {
    ULONG sigmask = extra_sigmask | eng->port_sigmask | quit_sigmask;

    // first run work loop of engine
    // end the loop if no more work has to be done or if signals have arrived
    D(("work loop\n"));
    while(1) {
      BOOL done = pamela_sock_work(eng);
      if(done) {
        break;
      }
      ULONG cur_mask = SetSignal(0,0);
      if((cur_mask & sigmask)!=0) {
        break;
      }
    }

    // wait for pamela event, port or timeout
    D(("event wait: sigmask=%lx\n", sigmask));
    int res = pamela_event_wait(eng->pamela, TIMEOUT_S, TIMEOUT_US, &sigmask);

    // timeout?
    if((res & PAMELA_WAIT_TIMEOUT) == PAMELA_WAIT_TIMEOUT) {
      D(("-> time out!\n"));
      pamela_sock_timeout(eng);
    }

    // pamela event?
    if((res & PAMELA_WAIT_EVENT) == PAMELA_WAIT_EVENT) {
      D(("-> pam event!\n"));
      pamela_sock_event(eng);
    }

    // some signal was hit
    if((res & PAMELA_WAIT_SIGMASK) == PAMELA_WAIT_SIGMASK) {
      // an ioreq was passed in
      if((sigmask & eng->port_sigmask) == eng->port_sigmask) {
        pamela_req_t *req = (pamela_req_t *)GetMsg(eng->req_port);
        D(("-> got req: %lx\n", req));
        if(req != NULL) {
          pamela_req_handle(eng, req);
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

  D(("init_request: req=%lx -> client=%lx\n", req, pc));
  AddTail((struct List *)&eng->clients, (struct Node *)pc);

  pc->request = req;
  pc->channel_mask = 0;

  req->iopam_Internal = pc;

  D(("init_request: done\n"));
  return PAMELA_OK;
}

int pamela_engine_exit_request(pamela_engine_t *eng, pamela_req_t *req)
{
  pamela_client_t *pc = (pamela_client_t *)req->iopam_Internal;
  if(pc == NULL) {
    return PAMELA_ERROR_CHANNEL_NOT_FOUND;
  }

  D(("exit_request: req=%lx -> client=%lx\n", req, pc));
  pamela_sock_shutdown_client(eng, pc);

  Remove((struct Node *)pc);

  FreeMem(pc, sizeof(pamela_client_t));

  D(("exit_request: done\n"));
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
      req->iopam_Req.io_Flags &= ~IOF_QUICK;
      req->iopam_Req.io_Error = 0;
      PutMsg(eng->req_port, (struct Message *)req);
      return FALSE;
    default:
      req->iopam_Req.io_Error = IOERR_NOCMD;
      req->iopam_Req.io_Flags |= IOF_QUICK;
      return TRUE;
  }
}

BOOL pamela_engine_cancel_request(pamela_engine_t *eng, pamela_req_t *req)
{
  // TODO
  req->iopam_Req.io_Error = IOERR_ABORTED;
  return FALSE;
}
