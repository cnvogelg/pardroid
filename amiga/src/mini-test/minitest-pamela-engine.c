#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pamela.h"
#include "pamela_engine.h"
#include "devices/pamela.h"

#define PORT 1234
#define SIZE 6

static void handle_error(int error)
{
  if(error != PAMELA_OK) {
    Printf("pamela error: %ld %s\n", error, pamela_perror(error));
  } else {
    PutStr("OK\n");
  }
}

static int handle_req(pamela_engine_t *eng, struct IOPamReq *req, struct MsgPort *port)
{
  Printf("  Post Request: cmd=%ld\n", req->iopam_Req.io_Command);
  BOOL quick = pamela_engine_post_request(eng, req);
  if(quick) {
    int error = req->iopam_Req.io_Error;
    Printf("  returned quick: error=%ld\n", error);
    return error;
  } else {
    // let the engine work
    ULONG extra_sigmask = 1 << port->mp_SigBit;
    PutStr("  engine works...\n");
    ULONG sigmask = pamela_engine_work(eng, extra_sigmask);

    if(sigmask != extra_sigmask) {
      Printf("  Returned wrong sigmask: %08lx\n", sigmask);
      return -1;
    }

    // get request back
    struct IOPamReq *res_req = (struct IOPamReq *)GetMsg(port);
    if(res_req != req) {
      Printf("  Returned wrong request! %08lx\n", res_req);
      return -2;
    }

    int error = req->iopam_PamelaError;
    Printf("  returned via msg: error=%ld\n", error);
    return error;
  }
}

int dosmain(void)
{
  pamela_engine_t *eng;
  int error;
  UBYTE buf[128];

  PutStr("test-pamela-engine\n");

  PutStr("engine init\n");
  eng = pamela_engine_init((struct Library *)SysBase, &error);
  handle_error(error);
  if(eng != NULL) {
    // create my message port
    struct MsgPort *port = CreateMsgPort();
    // allocate ioreq
    struct IOPamReq *req = (struct IOPamReq *)CreateIORequest(port, sizeof(*req));

    PutStr("init request\n");
    int res = pamela_engine_init_request(eng, req);
    handle_error(res);

    // open channel
    PutStr("open channel\n");
    req->iopam_Req.io_Command = PAMCMD_OPEN_CHANNEL;
    req->iopam_Port = 1234;
    res = handle_req(eng, req, port);
    handle_error(res);

    // reset
    PutStr("reset channel\n");
    req->iopam_Req.io_Command = PAMCMD_RESET_CHANNEL;
    res = handle_req(eng, req, port);
    handle_error(res);

    // seek
    PutStr("seek\n");
    req->iopam_Req.io_Command = PAMCMD_SEEK;
    req->iopam_Req.io_Offset = 0xcafebabe;
    res = handle_req(eng, req, port);
    handle_error(res);

    // tell
    PutStr("tell\n");
    req->iopam_Req.io_Command = PAMCMD_TELL;
    res = handle_req(eng, req, port);
    handle_error(res);
    Printf("pos=%lx\n", req->iopam_Req.io_Offset);

    // read
    PutStr("read\n");
    req->iopam_Req.io_Command = PAMCMD_READ;
    req->iopam_Req.io_Length = 128;
    req->iopam_Req.io_Data = buf;
    res = handle_req(eng, req, port);
    handle_error(res);
    Printf("actual=%ld\n", req->iopam_Req.io_Actual);

    // write
    PutStr("write\n");
    req->iopam_Req.io_Command = PAMCMD_WRITE;
    req->iopam_Req.io_Length = 128;
    req->iopam_Req.io_Data = buf;
    res = handle_req(eng, req, port);
    handle_error(res);
    Printf("actual=%ld\n", req->iopam_Req.io_Actual);

    // close channel
    PutStr("close channel\n");
    req->iopam_Req.io_Command = PAMCMD_CLOSE_CHANNEL;
    res = handle_req(eng, req, port);
    handle_error(res);

    PutStr("exit request\n");
    res = pamela_engine_exit_request(eng, req);
    handle_error(res);

    DeleteIORequest(req);
    DeleteMsgPort(port);

    PutStr("engine exit\n");
    pamela_engine_exit(eng);
    PutStr("done\n");
  }

  return 0;
}
