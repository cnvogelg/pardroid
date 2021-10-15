#ifndef PARIO_PORT_H
#define PARIO_PORT_H

#include "sim_msg.h"

struct pario_handle;

struct pario_port {
  struct pario_handle  *handle;
};

struct pario_handle {
  struct Library *      sysBase;

  sim_msg_handle_t      hnd_cmd;

  struct Task *         recv_task;
  struct Task *         sig_task;
  BYTE                  signal;
  BYTE                  sent_signal;
  UWORD                 sig_cnt;
  UWORD                 irq_cnt;

  struct pario_port     port;
};

#endif
