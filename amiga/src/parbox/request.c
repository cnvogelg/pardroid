#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "request.h"

request_t *request_create(struct MsgPort *reply_port)
{
  request_t *req = (request_t *)AllocMem(sizeof(request_t), MEMF_CLEAR | MEMF_PUBLIC);
  if(req != NULL) {
    req->msg.mn_ReplyPort = reply_port;
    req->msg.mn_Length = sizeof(request_t);
  }
  return req;
}

void request_delete(request_t *req)
{
  if(req == NULL) {
    return;
  }
  FreeMem(req, sizeof(request_t));
}
