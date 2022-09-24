#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "types.h"
#include "arch.h"

#include "pamlib.h"
#include "pamlib_req.h"

struct pamlib_req_int {
  pamlib_req_t pamlib_req;
  pamlib_channel_t *channel;
};
typedef struct pamlib_req_int pamlib_req_int_t;

#undef SysBase
#define SysBase pamlib_get_sysbase(ph)

static UBYTE *buffer_alloc(pamlib_channel_t *ch, UBYTE size)
{
  pamlib_handle_t *ph = pamlib_get_handle(ch);
  return AllocVec(size, MEMF_PUBLIC | MEMF_CLEAR);
}

static void buffer_free(pamlib_channel_t *ch, UBYTE *buf)
{
  if(buf == NULL) {
    return;
  }

  pamlib_handle_t *ph = pamlib_get_handle(ch);
  FreeVec(buf);
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

  UWORD size = mtu;

  pamlib_req_int_t *pc = AllocVec(sizeof(pamlib_req_int_t), MEMF_PUBLIC | MEMF_CLEAR);
  if(pc == NULL) {
    *error = PAMELA_ERROR_NO_MEM;
    return NULL;
  }

  pc->pamlib_req.buf = buffer_alloc(ch, size);
  if(pc->pamlib_req.buf == NULL) {
    FreeVec(pc);
    *error = PAMELA_ERROR_NO_MEM;
    return NULL;
  }

  pc->pamlib_req.mtu = mtu;
  pc->channel = ch;

  return &pc->pamlib_req;
}

int pamlib_req_close(pamlib_req_t *pamlib_req)
{
  if(pamlib_req == NULL) {
    return PAMELA_ERROR_NO_HANDLE;
  }

  pamlib_req_int_t *pc = (pamlib_req_int_t *)pamlib_req;
  pamlib_channel_t *ch = pc->channel;
  pamlib_handle_t *ph = pamlib_get_handle(ch);

  buffer_free(ch, pc->pamlib_req.buf);

  int res = pamlib_close(ch);

  FreeVec(pc);

  return res;
}

int pamlib_req_transfer(pamlib_req_t *cmd)
{
  // check parameter
  if(cmd == NULL) {
    return PAMELA_ERROR_NO_HANDLE;
  }
  pamlib_req_int_t *pc = (pamlib_req_int_t *)cmd;

  /* arg too long? */
  if(cmd->tx_size > cmd->mtu) {
    return PAMELA_ERROR_MSG_TOO_LARGE;
  }
  /* odd size? */
  if((cmd->tx_size & 1) == 1) {
    return PAMELA_ERROR_PROTO_ODD_SIZE;
  }

  /* write request of command */
  int res = pamlib_write(pc->channel, cmd->buf, cmd->tx_size);
  if(res < cmd->tx_size) {
    return PAMELA_ERROR_WRITE_FAILED;
  }

  /* receive result */
  return pamlib_read(pc->channel, cmd->buf, cmd->mtu);
}
