#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "parbox.h"

int parbox_init(parbox_handle_t *pb, struct Library *SysBase)
{
  pb->pario = NULL;
  pb->timer = NULL;
  pb->proto = NULL;
  pb->port = NULL;

  pb->pario = pario_init(SysBase);
  if(pb->pario == NULL) {
    return PARBOX_ERROR_PARIO;
  }

  pb->port = pario_get_port(pb->pario);

  pb->timer = timer_init(SysBase);
  if(pb->timer == NULL) {
    return PARBOX_ERROR_TIMER;
  }

  pb->proto = proto_init(pb->port, pb->timer, SysBase);
  if(pb->proto == NULL) {
    return PARBOX_ERROR_PROTO;
  }

  status_init(&pb->status);

  return PARBOX_OK;
}

void parbox_exit(parbox_handle_t *pb)
{
  if(pb->proto != NULL) {
    proto_exit(pb->proto);
    pb->proto = NULL;
  }

  if(pb->timer != NULL) {
    timer_exit(pb->timer);
    pb->timer = NULL;
  }

  if(pb->pario != NULL) {
    pario_exit(pb->pario);
    pb->pario = NULL;
  }
}

const char *parbox_perror(int res)
{
  switch(res) {
    case PARBOX_OK:
      return "parbox: OK";
    case PARBOX_ERROR_PARIO:
      return "parbox: pario failed!";
    case PARBOX_ERROR_TIMER:
      return "parbox: timer failed!";
    case PARBOX_ERROR_PROTO:
      return "parbox: proto failed!";
    default:
      return "parbox: unknown error!";
  }
}
