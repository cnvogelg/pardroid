#define __NOLIBBASE__
#include <proto/exec.h>
#include <libraries/bsdsocket.h>
#include <proto/bsdsocket.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_PARIO
#define KDEBUG
#endif

#include "compiler.h"
#include "debug.h"

#include "pario.h"
#include "pario_port.h"
#include "udp.h"

#define MY_PORT 2000
#define PEER_PORT 2001
#define BUF_SIZE 32768

struct pario_handle *pario_init(struct Library *SysBase)
{
  /* alloc handle */
  struct pario_handle *ph;
  ph = AllocMem(sizeof(struct pario_handle), MEMF_CLEAR | MEMF_PUBLIC);
  if(ph == NULL) {
    return NULL;
  }
  ph->sysBase = SysBase;
  ph->my_sock_fd = -1;
  ph->peer_sock_fd = -1;
  /* store ref to myself in fake port */
  ph->port.handle = ph;

  /* allocate message buffer */
  ph->msg_max = BUF_SIZE;
  ph->msg_buf = AllocMem(ph->msg_max, MEMF_CLEAR);
  if(ph->msg_buf == NULL) {
    D(("udp_init: no mem!\n"))
    goto init_failed;
  }

  /* setup udp */
  struct udp_handle *udp = &ph->udp_handle;
  if(udp_init(udp,(struct ExecBase *)SysBase)!=0) {
    D(("udp: init failed!\n"));
    goto init_failed;
  }

  /* setup my addr */
  if(udp_addr_setup(udp, &ph->my_addr, "0.0.0.0", MY_PORT)!=0) {
    D(("udp: setup my addr failed\n"));
    goto init_failed;
  }
  /* setup peer addr */
  if(udp_addr_setup(udp, &ph->peer_addr, "localhost", PEER_PORT)!=0) {
    D(("udp: setup peer port failed\n"));
    goto init_failed;
  }

  /* setup my bound socket */
  ph->my_sock_fd = udp_open(udp, &ph->my_addr);
  if(ph->my_sock_fd < 0) {
    D(("open my socket failed\n"));
    goto init_failed;
  }
  /* setup inbound peer socket */
  ph->peer_sock_fd = udp_open(udp, NULL);
  if(ph->peer_sock_fd < 0) {
    D(("open peer socket failed\n"));
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

  /* udp shutdown */
  struct udp_handle *udp = &ph->udp_handle;
  if(ph->my_sock_fd >= 0) {
    udp_close(udp, ph->my_sock_fd);
  }
  if(ph->peer_sock_fd >= 0) {
    udp_close(udp, ph->peer_sock_fd);
  }

  if(udp->socketBase != NULL) {
    udp_exit(udp);
  }

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
  return 0;
}

void pario_cleanup_ack_irq(struct pario_handle *ph)
{
  // nothing to do
}

struct pario_port *pario_get_port(struct pario_handle *ph)
{
  return &ph->port;
}

UWORD pario_get_ack_irq_counter(struct pario_handle *ph)
{
  return 0;
}

UWORD pario_get_signal_counter(struct pario_handle *ph)
{
   return 0;
}

void pario_confirm_ack_irq(struct pario_handle *ph)
{
}

// UDP helper

