#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "engine.h"
#include "worker.h"
#include "parbox.h"

struct engine_handle {
  worker_def_t      worker_def;
  parbox_handle_t   parbox;
  struct Library   *sys_base;
  int               result;
};

static BOOL startup(void *user_data, ULONG *user_sig_mask)
{
  engine_handle_t *eh = (engine_handle_t *)user_data;

  /* setup parbox */
  parbox_handle_t *ph = &eh->parbox;
  eh->result = parbox_init(ph, eh->sys_base);

  /* setup signal-based timer */
  struct timer_handle *th = ph->timer;
  if(timer_sig_init(th) == -1) {
    parbox_exit(ph);
    eh->result = PARBOX_ERROR_TIMER;
    return FALSE;
  }

  /* store timer sigmask */
  *user_sig_mask = timer_sig_get_mask(th);

  return eh->result == PARBOX_OK;
}

static void shutdown(void *user_data)
{
  engine_handle_t *eh = (engine_handle_t *)user_data;
  parbox_handle_t *ph = &eh->parbox;

  /* cleanup signal-based timer */
  struct timer_handle *th = ph->timer;
  timer_sig_exit(th);

  /* cleanup parbox */
  parbox_exit(ph);
}

static BOOL handle_msg(struct Message *msg, void *user_data)
{
  engine_handle_t *eh = (engine_handle_t *)user_data;
  /* TODO */
  return TRUE;
}

static void handle_sig(ULONG sig_mask, void *user_data)
{
  engine_handle_t *eh = (engine_handle_t *)user_data;
  /* TODO */
}

engine_handle_t *engine_start(int *result, struct Library *sys_base)
{
  engine_handle_t *eh;

  eh = AllocMem(sizeof(struct engine_handle), MEMF_CLEAR | MEMF_PUBLIC);
  if(eh == NULL) {
    return NULL;
  }

  /* setup worker task */
  worker_def_t *wd = &eh->worker_def;
  wd->task_name = "parbox_engine";
  wd->user_data = eh;
  wd->startup = startup;
  wd->shutdown = shutdown;
  wd->handle_msg = handle_msg;
  wd->handle_sig = handle_sig;

  eh->sys_base = sys_base;

  /* try to start worker */
  BOOL ok = worker_start(wd);
  *result = eh->result;
  if(!ok) {
    FreeMem(eh, sizeof(struct engine_handle));
    return NULL;
  } else {
    return eh;
  }
}

void engine_stop(engine_handle_t *eh)
{
  if(eh == NULL) {
    return;
  }

  /* stop worker */
  worker_def_t *wd = &eh->worker_def;
  worker_stop(wd);

  /* free handle */
  FreeMem(eh, sizeof(struct engine_handle));
}
