#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>
#include <exec/types.h>

#include "autoconf.h"
#include "debug.h"
#include "worker.h"

struct my_data {
  ULONG flags;
  struct Library *sysbase;
  BYTE signal;
};
typedef struct my_data my_data_t;

static BOOL startup(void *user_data, ULONG *user_sig_mask)
{
  my_data_t *my = (my_data_t *)user_data;
  my->flags |= 1;

  struct Library *SysBase = my->sysbase;
  my->signal = AllocSignal(-1);
  if(my->signal != -1) {
    *user_sig_mask = 1 << my->signal;
  }

  return TRUE;
}

static void shutdown(void *user_data)
{
  my_data_t *my = (my_data_t *)user_data;
  my->flags |= 2;

  struct Library *SysBase = my->sysbase;
  FreeSignal(-1);
}

static BOOL handle_msg(struct Message *msg, void *user_data)
{
  my_data_t *my = (my_data_t *)user_data;
  my->flags |= 4;
  return TRUE;
}

static void handle_sig(ULONG mask, void *user_data)
{
  my_data_t *my = (my_data_t *)user_data;
  my->flags |= 8;
}

int dosmain(void)
{
  my_data_t my = {
    .flags = 0,
    .sysbase = (struct Library *)SysBase
  };
  PutStr("testing worker...\n");
  worker_def_t def = {
    .task_name = "test_worker",
    .user_data = &my,
    .user_sig_mask = 0,
    .startup = startup,
    .shutdown = shutdown,
    .handle_msg = handle_msg,
    .handle_sig = handle_sig
  };
  BOOL ok = worker_start(&def);
  if(ok) {
    PutStr("started ok!\n");
    Printf("user sig mask: %08lx\n", def.user_sig_mask);

    struct MsgPort *port = CreateMsgPort();
    if(port != NULL) {
      PutStr("send message\n");
      struct Message msg;
      msg.mn_ReplyPort = port;
      msg.mn_Length = sizeof(struct Message);
      PutMsg(def.port, &msg);
      PutStr("wait for reply message\n");
      Wait(1 << port->mp_SigBit);
      DeleteMsgPort(port);
    }

    if(my.signal != -1) {
      PutStr("send signal\n");
      Signal(def.worker_task, def.user_sig_mask);
    }

    worker_stop(&def);
    PutStr("stopped.\n");
    Printf("Flags: %08lx\n", my.flags);
  } else {
    PutStr("failed to start!\n");
  }
  return 0;
}
