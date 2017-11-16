#ifndef PARBOX_H
#define PARBOX_H

#include "pario.h"
#include "timer.h"
#include "proto.h"
#include "status.h"

/* parbox init error codes */
#define PARBOX_OK             0
#define PARBOX_ERROR_PARIO    1
#define PARBOX_ERROR_TIMER    2
#define PARBOX_ERROR_PROTO    3

struct parbox_handle {
  struct pario_handle *pario;
  struct timer_handle *timer;
  struct proto_handle *proto;
  struct pario_port *port;
  status_data_t  status;
};

typedef struct parbox_handle parbox_handle_t;

int parbox_init(parbox_handle_t *ph, struct Library *SysBase);
void parbox_exit(parbox_handle_t *ph);

const char *parbox_perror(int res);

#endif /* PARBOX_H */
