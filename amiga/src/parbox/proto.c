#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "proto.h"
#include "proto_low.h"

/* proto signals */
#define clk_mask    pout_mask
#define rak_mask    busy_mask
#define cflg_mask   sel_mask

#define DDR_DATA_OUT  0xff
#define DDR_DATA_IN   0x00

struct proto_handle {
    struct pario_port   *port;
    struct timer_handle *timer;
    ULONG                timeout_s;
    ULONG                timeout_ms;
    struct Library      *sys_base;
};

proto_handle_t *proto_init(struct pario_port *port, struct timer_handle *th, struct Library *SysBase)
{
  proto_handle_t *ph;

  ph = AllocMem(sizeof(struct proto_handle), MEMF_CLEAR | MEMF_PUBLIC);
  if(ph == NULL) {
    return NULL;
  }
  ph->port = port;
  ph->timer = th;
  ph->timeout_s  = 0;
  ph->timeout_ms = 500000UL;
  ph->sys_base = SysBase;

  /* control: clk,cflg=out(1) rak=in*/
  *port->ctrl_ddr |= port->clk_mask | port->cflg_mask;
  *port->ctrl_ddr &= ~(port->rak_mask);
  *port->ctrl_port |= port->all_mask;

  /* data: port=0, ddr=0xff (OUT) */
  *port->data_port = 0;
  *port->data_ddr  = 0x0f; // lower nybble is for command bits 0..3

  return ph;
}

#undef SysBase
#define SysBase ph->sys_base

void proto_exit(proto_handle_t *ph)
{
  if(ph == NULL) {
    return;
  }
  /* free handle */
  FreeMem(ph, sizeof(struct proto_handle));
}

UBYTE proto_get_status(proto_handle_t *ph)
{
  struct pario_port *port = ph->port;
  return proto_low_get_status(port);
}

int proto_action(proto_handle_t *ph, UBYTE num)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  if(num > PROTO_MAX_ACTION) {
    return PROTO_RET_INVALID_ACTION;
  }
  UBYTE cmd = PROTO_CMD_ACTION + num;

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_action(port, timeout_flag, cmd);
  timer_stop(ph->timer);

  return result;
}

static ASM void bench_cb(REG(d0, int id), REG(a2, struct cb_data *cb))
{
  struct timer_handle *th = (struct timer_handle *)cb->user_data;
  time_stamp_t *ts = &cb->timestamps[id];
  timer_eclock_get(th, ts);
}

int proto_action_bench(proto_handle_t *ph, UBYTE num, time_stamp_t *start, ULONG deltas[2])
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  if(num > PROTO_MAX_ACTION) {
    return PROTO_RET_INVALID_ACTION;
  }
  UBYTE cmd = PROTO_CMD_ACTION + num;

  struct cb_data cbd = {
    bench_cb,
    ph->timer
  };

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_action_bench(port, timeout_flag, &cbd, cmd);
  timer_stop(ph->timer);

  /* calc deltas */
  time_stamp_t *t0 = &cbd.timestamps[0];
  time_stamp_t *t1 = &cbd.timestamps[1];
  time_stamp_t *t2 = &cbd.timestamps[2];
  time_stamp_t d1;
  time_stamp_t d2;
  timer_eclock_delta(t1, t0, &d1);
  timer_eclock_delta(t2, t1, &d2);

  *start = *t0;
  ULONG dummy;
  timer_eclock_split(&d1, &dummy, &deltas[0]);
  timer_eclock_split(&d2, &dummy, &deltas[1]);

  return result;
}

int proto_function_read(proto_handle_t *ph, UBYTE num, UWORD *data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  if(num > PROTO_MAX_FUNCTION) {
    return PROTO_RET_INVALID_FUNCTION;
  }
  UBYTE cmd = PROTO_CMD_FUNCTION + num;

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_read_word(port, timeout_flag, cmd, data);
  timer_stop(ph->timer);

  return result;
}

int proto_function_write(proto_handle_t *ph, UBYTE num, UWORD data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  if(num > PROTO_MAX_FUNCTION) {
    return PROTO_RET_INVALID_FUNCTION;
  }
  UBYTE cmd = PROTO_CMD_FUNCTION + num;

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_write_word(port, timeout_flag, cmd, &data);
  timer_stop(ph->timer);

  return result;
}

int proto_function_read_long(proto_handle_t *ph, UBYTE num, ULONG *data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  if(num > PROTO_MAX_FUNCTION) {
    return PROTO_RET_INVALID_FUNCTION;
  }
  UBYTE cmd = PROTO_CMD_FUNCTION + num;

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_read_long(port, timeout_flag, cmd, data);
  timer_stop(ph->timer);

  return result;
}

int proto_function_write_long(proto_handle_t *ph, UBYTE num, ULONG data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  if(num > PROTO_MAX_FUNCTION) {
    return PROTO_RET_INVALID_FUNCTION;
  }
  UBYTE cmd = PROTO_CMD_FUNCTION + num;

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_write_long(port, timeout_flag, cmd, &data);
  timer_stop(ph->timer);

  return result;
}

int proto_msg_write(proto_handle_t *ph, UBYTE chn, ULONG *msgiov)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  UBYTE cmd = chn + PROTO_CMD_MSG_WRITE;
  if(chn >= PROTO_MAX_CHANNEL) {
    return PROTO_RET_INVALID_CHANNEL;
  }

  if(msgiov[0] > 0xffff) {
    return PROTO_RET_MSG_TOO_LARGE;
  }

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_write_block(port, timeout_flag, cmd, msgiov);
  timer_stop(ph->timer);

  return result;
}

int proto_msg_read(proto_handle_t *ph, UBYTE chn, ULONG *msgiov)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  UBYTE cmd = chn + PROTO_CMD_MSG_READ;
  if(chn >= PROTO_MAX_CHANNEL) {
    return PROTO_RET_INVALID_CHANNEL;
  }

  if(msgiov[0] > 0xffff) {
    return PROTO_RET_MSG_TOO_LARGE;
  }

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_read_block(port, timeout_flag, cmd, msgiov);
  timer_stop(ph->timer);

  return result;
}

int proto_msg_write_single(proto_handle_t *ph, UBYTE chn, UBYTE *buf, UWORD num_words)
{
  ULONG msgiov[] = {
    num_words, /* total size */
    num_words, /* chunk size */
    (ULONG)buf,/* chunk pointer */
    0          /* last chunk */
  };
  return proto_msg_write(ph, chn, msgiov);
}

int proto_msg_read_single(proto_handle_t *ph, UBYTE chn, UBYTE *buf, UWORD *max_words)
{
  ULONG msgiov[] = {
    *max_words, /* total size */
    *max_words, /* chunk size */
    (ULONG)buf,/* chunk pointer */
    0          /* last chunk */
  };
  int result = proto_msg_read(ph, chn, msgiov);
  /* store returned result size */
  *max_words = (UWORD)(msgiov[0] & 0xffff);
  return result;
}

const char *proto_perror(int res)
{
  switch(res & PROTO_RET_MASK) {
    case PROTO_RET_OK:
      return "OK";
    case PROTO_RET_RAK_INVALID:
      return "RAK invalid";
    case PROTO_RET_TIMEOUT:
      return "timeout";
    case PROTO_RET_SLAVE_ERROR:
      return "slave error";
    case PROTO_RET_INVALID_FUNCTION:
      return "invalid function";
    case PROTO_RET_INVALID_CHANNEL:
      return "invalid channel";
    case PROTO_RET_INVALID_ACTION:
      return "invalid action";
    case PROTO_RET_MSG_TOO_LARGE:
      return "message too large";
    case PROTO_RET_WRITE_ABORT:
      return "write was aborted";
    case PROTO_RET_SLAVE_NOSPACE:
      return "slave has no space";
    default:
      return "?";
  }
}
