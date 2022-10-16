#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "types.h"
#include "arch.h"

#include "pamlib.h"
#include "pamlib_req.h"

struct pamlib_req{
  pamlib_channel_t *channel;
  UWORD mtu;
};

#undef SysBase
#define SysBase pamlib_get_sysbase(ph)

UWORD pamlib_req_get_max_size(pamlib_req_t *req)
{
  return req->mtu;
}

pamlib_req_t *pamlib_req_open(pamlib_handle_t *ph, UWORD port, int *error)
{
  /* first open channel */
  pamlib_channel_t *ch = pamlib_open(ph, port, error);
  if(ch == NULL) {
    return NULL;
  }

  /* get mtu to find out buffer sizes required */
  UWORD mtu = 0;
  int res = pamlib_get_mtu(ch, &mtu);
  if(res != PAMELA_OK) {
    pamlib_close(ch);
    *error = res;
    return NULL;
  }

  /* alloc req */
  pamlib_req_t *req = AllocVec(sizeof(pamlib_req_t), MEMF_PUBLIC | MEMF_CLEAR);
  if(req == NULL) {
    *error = PAMELA_ERROR_NO_MEM;
    return NULL;
  }

  req->mtu = mtu;
  req->channel = ch;

  return req;
}

int pamlib_req_close(pamlib_req_t *req)
{
  if(req == NULL) {
    return PAMELA_ERROR_NO_HANDLE;
  }

  pamlib_channel_t *ch = req->channel;
  pamlib_handle_t *ph = pamlib_get_handle(ch);
  int res = pamlib_close(ch);

  FreeVec(req);

  return res;
}

int pamlib_req_handle(pamlib_req_t *req, UBYTE *buf, UWORD req_size, UWORD *rep_size)
{
  // check parameter
  if(req == NULL) {
    return PAMELA_ERROR_NO_HANDLE;
  }

  /* arg too long? */
  if(req_size > req->mtu) {
    return PAMELA_ERROR_MSG_TOO_LARGE;
  }

  /* write request of command */
  int res = pamlib_write(req->channel, buf, req_size);
  if(res < 0) {
    return res;
  }

  /* make sure all was sent */
  if(res != req_size) {
    return PAMELA_ERROR_WRITE_FAILED;
  }

  /* receive result */
  res = pamlib_read(req->channel, buf, req->mtu);
  if(res < 0) {
    return res;
  }

  *rep_size = res;
  return PAMELA_OK;
}
