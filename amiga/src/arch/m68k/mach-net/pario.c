#define __NOLIBBASE__
#include <proto/exec.h>
#include <libraries/bsdsocket.h>
#include <clib/alib_protos.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_PARIO
#define KDEBUG
#endif

#include "compiler.h"
#include "debug.h"

#include "pario.h"
#include "pario_port.h"
#include "sim_msg.h"


SAVEDS static void task_main(void);

static APTR *get_my_user_data(void)
{
  /* retrieve global sys base */
  struct Library *SysBase = *((struct Library **)4);
  struct Task *task = FindTask(NULL);
  return task->tc_UserData;
}

struct pario_handle *pario_init(struct Library *SysBase)
{
  /* alloc handle */
  struct pario_handle *ph;
  ph = AllocMem(sizeof(struct pario_handle), MEMF_CLEAR | MEMF_PUBLIC);
  if(ph == NULL) {
    return NULL;
  }
  ph->sysBase = SysBase;

  /* allocate message buffer */
  ph->msg_max = BUF_SIZE;
  ph->msg_buf = AllocMem(ph->msg_max, MEMF_CLEAR);
  if(ph->msg_buf == NULL) {
    D(("udp_init: no mem!\n"))
    goto init_failed;
  }

  /* alloc sim_msg for commands */
  int res = sim_msg_client_init(&ph->hnd_cmd);
  if(res < 0) {
    D(("sim_msg: cmd: failed: %ld\n", res));
    goto init_failed;
  }

  /* all ok */
  return ph;

init_failed:
  /* something failed */
  pario_exit(ph);
  return NULL;
}

#define SysBase ph->sysBase

void pario_exit(struct pario_handle *ph)
{
  if(ph == NULL) {
    return;
  }

  /* free sim_msg cmd */
  sim_msg_client_exit(&ph->hnd_cmd);

  /* msg buffer */
  if(ph->msg_buf != NULL) {
    FreeMem(ph->msg_buf, ph->msg_max);
  }

  /* free handle */
  FreeMem(ph, sizeof(struct pario_handle));
}

int pario_setup_ack_irq(struct pario_handle *ph, struct Task *sigTask, BYTE signal)
{
  ph->sig_task = sigTask;
  ph->signal = signal;
  SetSignal(0, 1 << ph->signal);

  /* setup status task */
  Forbid();
  ph->recv_task = CreateTask("pario_sim", 0, (CONST APTR)task_main, 8000);
  ph->recv_task->tc_UserData = ph;
  Permit();

  return 0;
}

void pario_cleanup_ack_irq(struct pario_handle *ph)
{
  /* kill status task */


  ph->sig_task = NULL;
  ph->signal = -1;
}

struct pario_port *pario_get_port(struct pario_handle *ph)
{
  return &ph->port;
}

UWORD pario_get_ack_irq_counter(struct pario_handle *ph)
{
  return ph->irq_cnt;
}

UWORD pario_get_signal_counter(struct pario_handle *ph)
{
   return ph->sig_cnt;
}

void pario_confirm_ack_irq(struct pario_handle *ph)
{
  ph->sent_signal = 0;
  SetSignal(0, 1 << ph->signal);
}

SAVEDS static void task_main(void)
{
  struct pario_handle *ph = (struct pario_handle *)get_my_user_data();
  D(("recv_main: %ld\n", ph));

  struct sim_msg_handle hnd;
  int res = sim_msg_client_init(&hnd);

  if(res==0) {

    /* main loop */
    ULONG status = 0;
    while(1) {
      // get next event
      res = sim_msg_client_do_status(&hnd, &status);
      if(res == 0) {

      } else {
        D(("sim_msg: status: do failed: %ld\n", res));
      }
    }

    sim_msg_client_exit(&hnd);
  } else {
    D(("sim_msg: status: init failed: %ld\n", res));
  }

  /* kill myself */
  D(("Task: die\n"));
  struct Task *me = FindTask(NULL);
  DeleteTask(me);
  Wait(0);
  D(("Task: NEVER!\n"));
}

