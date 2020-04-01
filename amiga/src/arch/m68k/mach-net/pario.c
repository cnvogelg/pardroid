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

  /* open bsdsocket.library */
  ph->socketBase = OpenLibrary("bsdsocket.library",0);
  if(ph->socketBase != NULL) {
    D(("opened bsdsocket.library\n"));

    /* setup my addr */
    if(udp_addr_setup(ph, &ph->my_addr, "localhost", MY_PORT)==0) {
      D(("setup my addr\n"));
      /* setup peer addr */
      if(udp_addr_setup(ph, &ph->peer_addr, "localhost", PEER_PORT)==0) {
        D(("setup peer port\n"));
        /* setup my bound socket */
        ph->my_sock_fd = udp_open(ph, &ph->my_addr);
        if(ph->my_sock_fd >= 0) {
          D(("opened my socket\n"));
          /* setup inbound peer socket */
          ph->peer_sock_fd = udp_open(ph, NULL);
          if(ph->peer_sock_fd != 0) {
            /* all ok! */
            return ph;
          }
        }
      }
    }
  }

  /* something failed */
  pario_exit(ph);
  return NULL;
}

#define SysBase ph->sysBase
#define SocketBase ph->socketBase

void pario_exit(struct pario_handle *ph)
{
  if(ph == NULL) {
    return;
  }

  if(ph->my_sock_fd >= 0) {
    udp_close(ph, ph->my_sock_fd);
  }
  if(ph->peer_sock_fd >= 0) {
    udp_close(ph, ph->peer_sock_fd);
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

