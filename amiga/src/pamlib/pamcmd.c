#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "types.h"
#include "arch.h"

#include "pamlib.h"
#include "pamcmd.h"

struct pamcmd_int {
  pamcmd_t pamcmd;
  UBYTE    *buf;
  UWORD     arg_max_size;
  UWORD     mtu;
  pamlib_channel_t *channel;
};
typedef struct pamcmd_int pamcmd_int_t;

#undef SysBase
#define SysBase pamlib_get_sysbase(ph)

static UBYTE *buffer_alloc(pamlib_channel_t *ch, UBYTE size)
{
  pamlib_handle_t *ph = pamlib_get_handle(ch);
  return AllocVec(size + PAMCMD_HEADER_SIZE, MEMF_PUBLIC | MEMF_CLEAR);
}

static void buffer_free(pamlib_channel_t *ch, UBYTE *buf)
{
  if(buf == NULL) {
    return;
  }

  pamlib_handle_t *ph = pamlib_get_handle(ch);
  FreeVec(buf);
}

pamcmd_t *pamcmd_open(pamlib_handle_t *ph, UWORD port, int *error)
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

  pamcmd_int_t *pc = AllocVec(sizeof(pamcmd_int_t), MEMF_PUBLIC | MEMF_CLEAR);
  if(pc == NULL) {
    *error = PAMELA_ERROR_NO_MEM;
    return NULL;
  }

  pc->buf = buffer_alloc(ch, size);
  if(pc->buf == NULL) {
    FreeVec(pc);
    *error = PAMELA_ERROR_NO_MEM;
    return NULL;
  }

  pc->mtu = mtu;
  pc->arg_max_size = mtu - PAMCMD_HEADER_SIZE;
  pc->channel = ch;

  pc->pamcmd.arg_buf = pc->buf + PAMCMD_HEADER_SIZE;

  return (pamcmd_t *)pc;
}

int pamcmd_close(pamcmd_t *pamcmd)
{
  if(pamcmd == NULL) {
    return PAMELA_ERROR_WRONG_PARAM;
  }

  pamcmd_int_t *pc = (pamcmd_int_t *)pamcmd;
  pamlib_channel_t *ch = pc->channel;
  pamlib_handle_t *ph = pamlib_get_handle(ch);

  buffer_free(ch, pc->buf);

  int res = pamlib_close(ch);

  FreeVec(pc);

  return res;
}

int pamcmd_transfer(pamcmd_t *cmd)
{
  // check parameter
  if(cmd == NULL) {
    return PAMELA_ERROR_WRONG_PARAM;
  }
  pamcmd_int_t *pc = (pamcmd_int_t *)cmd;

  /* arg too long? */
  if(cmd->tx_arg_size > pc->arg_max_size) {
    return PAMELA_ERROR_ARG_TOO_LONG;
  }

  /* fill header */
  UWORD size = cmd->tx_arg_size;
  UBYTE *buf = pc->buf;
  buf[0] = cmd->cmd_id;
  buf[1] = 0;
  buf[2] = (UBYTE)(size & 0xff);
  buf[3] = (UBYTE)(size >> 8);

  /* calc packet len */
  UWORD pkt_len = size + PAMCMD_HEADER_SIZE;
  if((pkt_len & 1)==1) {
    pkt_len++; // pad to word
  }

  /* write request of command */
  int res = pamlib_write(pc->channel, pc->buf, pkt_len);
  if(res < pkt_len) {
    return res;
  }

  /* receive result */
  res = pamlib_read(pc->channel, pc->buf, pc->mtu);
  if(res < 0) {
    return res;
  }

  /* no header? */
  if(res < PAMCMD_HEADER_SIZE) {
    return PAMELA_ERROR_COMMAND;
  }
  /* check command */
  if(pc->buf[0] != cmd->cmd_id) {
    return PAMELA_ERROR_COMMAND;
  }

  /* check len */
  UWORD arg_len = pc->buf[2] | pc->buf[3] << 8;
  pkt_len = arg_len + PAMCMD_HEADER_SIZE;
  if((pkt_len & 1)==1) {
    pkt_len++; // pad to word
  }
  if(pkt_len != res) {
    return PAMELA_ERROR_COMMAND;
  }

  /* check status */
  cmd->status = pc->buf[1];
  if(cmd->status == 0) {
    cmd->rx_arg_size = arg_len;
    return PAMELA_OK;
  } else {
    return PAMELA_ERROR_COMMAND;
  }
}
