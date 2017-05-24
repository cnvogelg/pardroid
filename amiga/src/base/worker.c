#include <proto/exec.h>
#include <proto/alib.h>

#include <exec/types.h>
#include <exec/ports.h>

#ifdef CONFIG_DEBUG_WORKER
#define KDEBUG
#endif

#define NO_SYSBASE
#include "autoconf.h"
#include "compiler.h"
#include "debug.h"
#include "worker.h"

static worker_def_t * worker_startup(void)
{
  /* retrieve global sys base */
  struct Library *SysBase = *((struct Library **)4);

  struct Task *task = FindTask(NULL);

  worker_def_t * def = (worker_def_t *)task->tc_UserData;
  def->worker_task = task;
  return def;
}

#define SysBase def->sys_base

static SAVEDS ASM void worker_main(void)
{
  /* retrieve dev base stored in user data of task */
  worker_def_t *def = worker_startup();
  D(("Task: def=%08lx\n", def));

  /* create worker port */
  struct MsgPort *port = CreateMsgPort();

  /* call user init */
  if(port != NULL) {
    D(("Task: user startup\n"));
    if(!def->startup(def->user_data, &def->user_sig_mask)) {
      /* user aborted worker */
      DeleteMsgPort(port);
      port = NULL;
    }
    D(("Task: user startup done: %08lx\n", port));
  }

  /* setup term signal */
  def->term_signal = AllocSignal(-1);
  if(def->term_signal == -1) {
    DeleteMsgPort(port);
    port = NULL;
  } else {
    def->term_sig_mask = 1 << def->term_signal;
  }

  /* setup port or NULL and trigger signal to caller task */
  def->port = port;
  D(("Task: signal task=%08lx mask=%08lx\n", def->main_task, def->ack_sig_mask));
  Signal(def->main_task, def->ack_sig_mask);

  /* only if port is available then enter work loop. otherwise quit task */
  if(port != NULL)
  {
    /* worker loop */
    D(("Task: enter\n"));
    BOOL stay = TRUE;
    ULONG port_mask = (1 << port->mp_SigBit);
    ULONG mask =  port_mask | def->user_sig_mask | def->term_sig_mask;
    while (stay) {
      ULONG got_mask = Wait(mask);
      /* user signal */
      if(got_mask & def->user_sig_mask) {
        D(("Task: user signal: %08lx\n", mask));
        def->handle_sig(mask, def->user_data);
      }
      /* handle messages */
      if(got_mask & port_mask) {
        D(("Task: got messages\n"));
        while (1) {
          struct Message *msg = GetMsg(port);
          if(msg == NULL) {
            break;
          } else {
            BOOL auto_reply = def->handle_msg(msg, def->user_data);
            /* auto reply message */
            if(auto_reply && msg->mn_ReplyPort != NULL) {
              ReplyMsg(msg);
            }
          }
        }
      }
      /* terminate signal */
      if(got_mask & def->term_sig_mask) {
        D(("Task: terminate\n"));
        stay = FALSE;
      }
    }

    /* call shutdown only if worker was entered */
    D(("Task: user shutdown\n"));
    /* shutdown worker */
    def->shutdown(def->user_data);
    D(("Task: user shutdown done\n"));
  }

  D(("Task: delete port\n"));
  DeleteMsgPort(port);
  def->port = NULL;

  D(("Task: free signal\n"));
  FreeSignal(def->term_signal);

  D(("Task: end ack signal\n"));
  Signal(def->main_task, def->ack_sig_mask);

  D(("Task: done\n"));
}

BOOL worker_start(worker_def_t *def)
{
  D(("Worker: start\n"));
  def->sys_base = *((struct Library **)4);
  def->port = NULL;

  /* alloc a signal */
  BYTE signal = AllocSignal(-1);
  if(signal == -1) {
    D(("Worker: NO SIGNAL!\n"));
    return FALSE;
  }

  /* setup init data */
  def->ack_sig_mask = 1 << signal;
  def->ack_signal = signal;
  def->main_task = FindTask(NULL);
  D(("Worker: def %08lx\n", def));

  /* now launch worker task and inject dev base
     make sure worker_main() does not run before base is set.
  */
  Forbid();
  struct Task *myTask = CreateTask(def->task_name, 0, (CONST APTR)worker_main, 4096);
  if(myTask != NULL) {
    myTask->tc_UserData = (APTR)def;
  }
  Permit();
  if(myTask == NULL) {
    D(("Worker: NO TASK!\n"));
    FreeSignal(signal);
    return FALSE;
  }

  /* wait for start signal of new task */
  D(("Worker: wait for task startup. sigmask=%08lx\n", def->ack_sig_mask));
  Wait(def->ack_sig_mask);

  if(def->port) {
    /* ok everything is fine. worker is ready to receive commands */
    D(("Worker: started: port=%08lx\n", def->port));
    return TRUE;
  } else {
    D(("Worker: failed to start\n"));
    FreeSignal(signal);
    return FALSE;
  }
}

void worker_stop(worker_def_t *def)
{
  D(("Worker: stop\n"));

  if(def->port != NULL) {
    /* send term signal to new task */
    Signal(def->worker_task, def->term_sig_mask);

    /* wait for shutdown */
    D(("Worker: wait for ack signal\n"));
    Wait(def->ack_sig_mask);

    /* remove my signal */
    FreeSignal(def->ack_signal);
  }

  D(("Worker: stopped\n"));
}
