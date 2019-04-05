#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pamela.h"

struct pamela_handle {
  struct pario_handle *pario;
  struct timer_handle *timer;
  struct proto_handle *proto;
  struct pario_port *port;
  struct Library *sys_base;
  ULONG  ack_irq_sigmask;
  ULONG  timer_sigmask;
  BYTE   ack_irq_signal;
  BYTE   timer_signal;
};

pamela_handle_t *pamela_init(struct Library *SysBase, int *res, int flags)
{
  pamela_handle_t *ph = AllocMem(sizeof(pamela_handle_t), MEMF_CLEAR | MEMF_PUBLIC);
  if(ph == NULL) {
    return NULL;
  }

  ph->pario = NULL;
  ph->timer = NULL;
  ph->proto = NULL;
  ph->port = NULL;
  ph->sys_base = SysBase;

  ph->pario = pario_init(SysBase);
  if(ph->pario == NULL) {
    *res = PAMELA_ERROR_PARIO;
    return NULL;
  }

  ph->port = pario_get_port(ph->pario);

  ph->timer = timer_init(SysBase);
  if(ph->timer == NULL) {
    pario_exit(ph->pario);
    ph->pario = NULL;

    *res = PAMELA_ERROR_TIMER;
    return NULL;
  }

  ph->proto = proto_init(ph->port, ph->timer, SysBase);
  if(ph->proto == NULL) {
    pario_exit(ph->pario);
    ph->pario = NULL;
    timer_exit(ph->timer);
    ph->timer = NULL;

    *res = PAMELA_ERROR_PROTO;
    return NULL;
  }

  /* enter bootloader */
  if((flags & PAMELA_INIT_BOOT) == 0) {
    int pres = proto_bootloader(ph->proto);
    if(pres != PROTO_RET_OK) {
      *res = PAMELA_ERROR_BOOTLOADER;
      pamela_exit(ph);
      return NULL;
    }
  }
  /* reset device (leave knok) */
  else {
    int pres = proto_reset(ph->proto);
    if(pres != PROTO_RET_OK) {
      *res = PAMELA_ERROR_RESET;
      pamela_exit(ph);
      return NULL;
    }
  }

  *res = PAMELA_OK;
  return ph;
}

#undef SysBase
#define SysBase ph->sys_base

void pamela_exit(pamela_handle_t *ph)
{
  if(ph->proto != NULL) {
    proto_exit(ph->proto);
    ph->proto = NULL;
  }

  if(ph->timer != NULL) {
    timer_exit(ph->timer);
    ph->timer = NULL;
  }

  if(ph->pario != NULL) {
    pario_exit(ph->pario);
    ph->pario = NULL;
  }

  FreeMem(ph, sizeof(pamela_handle_t));
}

proto_handle_t *pamela_get_proto(pamela_handle_t *ph)
{
  return ph->proto;
}

timer_handle_t *pamela_get_timer(pamela_handle_t *ph)
{
  return ph->timer;
}

int pamela_init_events(pamela_handle_t *ph)
{
  ph->timer_signal = -1;

  /* alloc ack signal */
  ph->ack_irq_signal = AllocSignal(-1);
  if(ph->ack_irq_signal == -1) {
    return PAMELA_ERROR_NO_SIGNAL;
  }

  /* setup ack irq handler */
  struct Task *task = FindTask(NULL);
  int error = pario_setup_ack_irq(ph->pario, task, ph->ack_irq_signal);
  if(error) {
    FreeSignal(ph->ack_irq_signal);
    ph->ack_irq_signal = -1;
    return PAMELA_ERROR_ACK_IRQ;
  }

  /* setup signal timer */
  ph->timer_signal = timer_sig_init(ph->timer);
  if(ph->timer_signal == -1) {
    pario_cleanup_ack_irq(ph->pario);
    FreeSignal(ph->ack_irq_signal);
    ph->ack_irq_signal = -1;
    return PAMELA_ERROR_TIMER_SIG;
  }

  ph->ack_irq_sigmask = 1 << ph->ack_irq_signal;
  ph->timer_sigmask = 1 << ph->timer_signal;

  return PAMELA_OK;
}

void pamela_exit_events(pamela_handle_t *ph)
{
  if(ph->timer_signal != -1) {
    /* timer cleanup */
    timer_sig_exit(ph->timer);
    ph->timer_signal = -1;
  }

  if(ph->ack_irq_signal != -1) {
    /* cleanup ack irq */
    pario_cleanup_ack_irq(ph->pario);

    /* free signal */
    FreeSignal(ph->ack_irq_signal);
    ph->ack_irq_signal = -1;
  }
}

ULONG pamela_wait_event(pamela_handle_t *ph,
                        ULONG timeout_s, ULONG timeout_us, ULONG extra_sigmask)
{
  /* wait for either timeout or ack */
  ULONG ack_mask = ph->ack_irq_sigmask;
  ULONG mask = ack_mask | ph->timer_sigmask | extra_sigmask;
  timer_sig_start(ph->timer, timeout_s, timeout_us);
  ULONG got = Wait(mask);
  timer_sig_stop(ph->timer);

  // confirm ack irq
  if(got & ack_mask) {
    pario_confirm_ack_irq(ph->pario);
  }

  return got;
}

ULONG pamela_get_trigger_sigmask(pamela_handle_t *ph)
{
  return ph->ack_irq_sigmask;
}

ULONG pamela_get_timer_sigmask(pamela_handle_t *ph)
{
  return ph->timer_sigmask;
}

UWORD pamela_get_num_triggers(pamela_handle_t *ph)
{
  return pario_get_ack_irq_counter(ph->pario);
}

UWORD pamela_get_num_trigger_signals(pamela_handle_t *ph)
{
  return pario_get_signal_counter(ph->pario);
}

const char *pamela_perror(int res)
{
  switch(res) {
    case PAMELA_OK:
      return "pamela: OK";
    case PAMELA_ERROR_PARIO:
      return "pamela: pario failed!";
    case PAMELA_ERROR_TIMER:
      return "pamela: timer failed!";
    case PAMELA_ERROR_PROTO:
      return "pamela: proto failed!";
    case PAMELA_ERROR_NO_SIGNAL:
      return "pamela: no signal";
    case PAMELA_ERROR_ACK_IRQ:
      return "pamela: ack irq";
    case PAMELA_ERROR_TIMER_SIG:
      return "pamela: timer sig";
    case PAMELA_ERROR_RESET:
      return "pamela: failed device reset";
    case PAMELA_ERROR_BOOTLOADER:
      return "pamela: failed entering device bootloader";
    default:
      return "pamela: unknown error!";
  }
}
