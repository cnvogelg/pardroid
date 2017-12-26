#ifndef PAMELA_H
#define PAMELA_H

#include "pario.h"
#include "timer.h"
#include "proto.h"
#include "status.h"

/* pamela init error codes */
#define PAMELA_OK             0
#define PAMELA_ERROR_PARIO    1
#define PAMELA_ERROR_TIMER    2
#define PAMELA_ERROR_PROTO    3

struct pamela_handle {
  struct pario_handle *pario;
  struct timer_handle *timer;
  struct proto_handle *proto;
  struct pario_port *port;
  status_data_t  status;
};

typedef struct pamela_handle pamela_handle_t;

int pamela_init(pamela_handle_t *ph, struct Library *SysBase);
void pamela_exit(pamela_handle_t *ph);

const char *pamela_perror(int res);

#endif /* PAMELA_H */
