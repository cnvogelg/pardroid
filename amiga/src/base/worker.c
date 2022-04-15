#define __NOLIBBASE__
#include <proto/exec.h>
#include <clib/alib_protos.h>

#include <exec/types.h>
#include <exec/errors.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_WORKER
#define KDEBUG
#endif

#include "compiler.h"
#include "debug.h"

#include "worker.h"

struct init_data
{
  ULONG callee_sigmask;
  struct Task *callee_task;
  APTR user_data;
  worker_init_func_t init_func;
  worker_main_func_t main_func;
  BOOL quit_signal;
  BOOL init_ok;
};

static SAVEDS ASM void worker_main(void)
{
  /* retrieve global sys base */
  struct Library *SysBase = *((struct Library **)4);
  struct Task *task = FindTask(NULL);
  struct init_data *id = (struct init_data *)task->tc_UserData;
  D(("WorkerTask: id=%08lx task=%08lx\n", id, FindTask(NULL)));

  /* trigger init func */
  BOOL init_ok = TRUE;
  APTR user_data = id->user_data;
  if(id->init_func != NULL) {
    init_ok = id->init_func(user_data);
  }

  /* copy all we need from init_data as it will be released
     after signal... */
  worker_main_func_t main_func = id->main_func;
  BOOL quit_signal = id->quit_signal;
  struct Task *callee_task = id->callee_task;
  ULONG callee_sigmask = id->callee_sigmask;

  /* report init state to caller */
  id->init_ok = init_ok;
  D(("WorkerTask: signal task=%08lx mask=%08lx\n", callee_task, callee_sigmask));
  Signal(callee_task, callee_sigmask);

  /* enter main only if init was ok */
  if (init_ok) {
    D(("WorkerTask: main\n"));
    if(main_func != NULL) {
      main_func(user_data);
    }
  }

  /* send quit signal? */
  if(quit_signal) {
    D(("WorkerTask: send quit signal\n"));
    Signal(callee_task, callee_sigmask);
  }

  /* kill myself */
  D(("WorkerTask: die\n"));
  struct Task *me = FindTask(NULL);
  DeleteTask(me);
  Wait(0);
  D(("WorkerTask: NEVER!\n"));
}

struct Task *worker_run(struct Library *SysBase, STRPTR task_name, ULONG stack_size,
  worker_init_func_t init_func,
  worker_main_func_t main_func,
  APTR user_data, BYTE *quit_signal)
{
  D(("Worker: start\n"));

  /* alloc a signal */
  BYTE signal = AllocSignal(-1);
  if (signal == -1)
  {
    D(("Worker: NO SIGNAL!\n"));
    return NULL;
  }

  /* setup init data */
  struct init_data id;
  id.callee_sigmask = 1 << signal;
  id.callee_task = FindTask(NULL);
  id.user_data = user_data;
  id.init_func = init_func;
  id.main_func = main_func;
  id.init_ok = FALSE;
  id.quit_signal = (quit_signal != NULL);
  D(("Worker: init data %08lx\n", &id));

  /* now launch worker task and inject init data
     make sure worker_main() does not run before base is set. */
  Forbid();
  struct Task *myTask = CreateTask(task_name, 0, (CONST APTR)worker_main, stack_size);
  if (myTask != NULL)
  {
    myTask->tc_UserData = (APTR)&id;
  }
  Permit();
  if (myTask == NULL)
  {
    D(("Worker: NO TASK!\n"));
    FreeSignal(signal);
    return NULL;
  }

  /* wait for start signal of new task */
  D(("Worker: wait for task startup. sigmask=%08lx\n", id.callee_sigmask));
  Wait(id.callee_sigmask);
  /* clear signal */
  SetSignal(0, id.callee_sigmask);

  /* keep signal as quit signal? */
  if(quit_signal != NULL) {
    *quit_signal = signal;
  } else {
    /* no need for signal anymore */
    FreeSignal(signal);
  }

  /* ok everything is fine. worker is ready to receive commands */
  D(("Worker: run done. ok=%ld", id.init_ok));
  if(id.init_ok) {
    return myTask;
  } else {
    return NULL;
  }
}

void worker_join(struct Library *SysBase, BYTE quit_signal)
{
  D(("Worker: join\n"));
  ULONG mask = 1 << quit_signal;
  Wait(mask);
  SetSignal(0, mask);
  FreeSignal(quit_signal);
  D(("Worker: join done\n"));
}

