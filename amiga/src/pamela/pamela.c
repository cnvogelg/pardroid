#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pamela.h"

int pamela_init(pamela_handle_t *pb, struct Library *SysBase)
{
  pb->pario = NULL;
  pb->timer = NULL;
  pb->proto = NULL;
  pb->port = NULL;

  pb->pario = pario_init(SysBase);
  if(pb->pario == NULL) {
    return PAMELA_ERROR_PARIO;
  }

  pb->port = pario_get_port(pb->pario);

  pb->timer = timer_init(SysBase);
  if(pb->timer == NULL) {
    return PAMELA_ERROR_TIMER;
  }

  pb->proto = proto_init(pb->port, pb->timer, SysBase);
  if(pb->proto == NULL) {
    return PAMELA_ERROR_PROTO;
  }

  status_init(&pb->status);

  return PAMELA_OK;
}

void pamela_exit(pamela_handle_t *pb)
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
    default:
      return "pamela: unknown error!";
  }
}
