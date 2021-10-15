#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "proto.h"
#include "proto_low.h"

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

  proto_low_config_port(port);

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

int proto_reset(proto_handle_t *ph)
{
  // perform reset action
  return proto_action_no_busy(ph, PROTO_ACTION_RESET);
}

int proto_bootloader(proto_handle_t *ph)
{
  // perform bootloader action
  return proto_action_no_busy(ph, PROTO_ACTION_BOOTLOADER);
}

int proto_ping(proto_handle_t *ph)
{
  // perform ping action
  return proto_action(ph, PROTO_ACTION_PING);
}

int proto_knok(proto_handle_t *ph)
{
  // perform knok action
  return proto_action_no_busy(ph, PROTO_ACTION_KNOK);
}

static int raw_action(proto_handle_t *ph, UBYTE cmd)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_action(port, timeout_flag, cmd);
  timer_stop(ph->timer);

  return result;
}

int proto_action(proto_handle_t *ph, UBYTE num)
{
  if(num > PROTO_MAX_ACTION) {
    return PROTO_RET_INVALID_ACTION;
  }
  UBYTE cmd = PROTO_CMD_ACTION + num;
  return raw_action(ph, cmd);
}

int proto_action_no_busy(proto_handle_t *ph, UBYTE num)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  if(num > PROTO_MAX_ACTION) {
    return PROTO_RET_INVALID_ACTION;
  }
  UBYTE cmd = PROTO_CMD_ACTION + num;

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_action_no_busy(port, timeout_flag, cmd);
  timer_stop(ph->timer);

  return result;
}

static ASM void bench_cb(REG(d0, int id), REG(a2, struct cb_data *cb))
{
  struct timer_handle *th = (struct timer_handle *)cb->user_data;
  time_stamp_t *ts = &cb->timestamps[id];
  timer_eclock_get(th, ts);
}

int proto_action_bench(proto_handle_t *ph, UBYTE num, ULONG deltas[2])
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
  time_stamp_t d0, d1;
  timer_eclock_delta(t1, t0, &d0);
  timer_eclock_delta(t2, t1, &d1);
  deltas[0] = timer_eclock_to_us(ph->timer, &d0);
  deltas[1] = timer_eclock_to_us(ph->timer, &d1);

  return result;
}

static int read_word(proto_handle_t *ph, UBYTE cmd, UWORD *data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_read_word(port, timeout_flag, cmd, data);
  timer_stop(ph->timer);

  return result;
}

static int write_word(proto_handle_t *ph, UBYTE cmd, UWORD data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_write_word(port, timeout_flag, cmd, &data);
  timer_stop(ph->timer);

  return result;
}

int proto_wfunc_read(proto_handle_t *ph, UBYTE num, UWORD *data)
{
  if(num > PROTO_MAX_FUNCTION) {
    return PROTO_RET_INVALID_FUNCTION;
  }
  UBYTE cmd = PROTO_CMD_WFUNC_READ + num;
  return read_word(ph, cmd, data);
}

int proto_wfunc_write(proto_handle_t *ph, UBYTE num, UWORD data)
{
  if(num > PROTO_MAX_FUNCTION) {
    return PROTO_RET_INVALID_FUNCTION;
  }
  UBYTE cmd = PROTO_CMD_WFUNC_WRITE + num;
  return write_word(ph, cmd, data);
}

static int read_long(proto_handle_t *ph, UBYTE cmd, ULONG *data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_read_long(port, timeout_flag, cmd, data);
  timer_stop(ph->timer);

  return result;
}

static int write_long(proto_handle_t *ph, UBYTE cmd, ULONG data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_write_long(port, timeout_flag, cmd, &data);
  timer_stop(ph->timer);

  return result;
}

int proto_lfunc_read(proto_handle_t *ph, UBYTE num, ULONG *data)
{
  if(num > PROTO_MAX_FUNCTION) {
    return PROTO_RET_INVALID_FUNCTION;
  }
  UBYTE cmd = PROTO_CMD_LFUNC_READ + num;
  return read_long(ph, cmd, data);
}

int proto_lfunc_write(proto_handle_t *ph, UBYTE num, ULONG data)
{
  if(num > PROTO_MAX_FUNCTION) {
    return PROTO_RET_INVALID_FUNCTION;
  }
  UBYTE cmd = PROTO_CMD_LFUNC_WRITE + num;
  return write_long(ph, cmd, data);
}

int proto_chn_msg_write(proto_handle_t *ph, UBYTE chn, UBYTE *buf, UWORD num_words)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  if(chn >= PROTO_MAX_CHANNEL) {
    return PROTO_RET_INVALID_CHANNEL;
  }
  UBYTE cmd = chn + PROTO_CMD_CHN_WRITE_DATA;

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_write_block(port, timeout_flag, cmd, buf, num_words);
  timer_stop(ph->timer);

  return result;
}

int proto_chn_msg_read(proto_handle_t *ph, UBYTE chn, UBYTE *buf, UWORD num_words)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  if(chn >= PROTO_MAX_CHANNEL) {
    return PROTO_RET_INVALID_CHANNEL;
  }
  UBYTE cmd = chn + PROTO_CMD_CHN_READ_DATA;

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_read_block(port, timeout_flag, cmd, buf, num_words);
  timer_stop(ph->timer);

  return result;
}

// extended commands

int proto_chn_get_rx_size(proto_handle_t *ph, UBYTE chn, UWORD *ret_words)
{
  if(chn > PROTO_MAX_CHANNEL) {
    return PROTO_RET_INVALID_CHANNEL;
  }
  UBYTE cmd = PROTO_CMD_CHN_GET_RX_SIZE + chn;
  return read_word(ph, cmd, ret_words);
}

int proto_chn_set_tx_size(proto_handle_t *ph, UBYTE chn, UWORD num_words)
{
  if(chn > PROTO_MAX_CHANNEL) {
    return PROTO_RET_INVALID_CHANNEL;
  }
  UBYTE cmd = PROTO_CMD_CHN_SET_TX_SIZE + chn;
  return write_word(ph, cmd, num_words);
}

int proto_chn_set_offset(proto_handle_t *ph, UBYTE chn, ULONG offset)
{
  if(chn > PROTO_MAX_CHANNEL) {
    return PROTO_RET_INVALID_CHANNEL;
  }
  UBYTE cmd = PROTO_CMD_CHN_SET_OFFSET + chn;
  return write_long(ph, cmd, offset);
}

int proto_chn_cancel_transfer(proto_handle_t *ph, UBYTE chn)
{
  if(chn > PROTO_MAX_CHANNEL) {
    return PROTO_RET_INVALID_CHANNEL;
  }
  UBYTE cmd = PROTO_CMD_CHN_CANCEL_TRANSFER + chn;
  return raw_action(ph, cmd);
}

// verbose error

const char *proto_perror(int res)
{
  switch(res) {
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
    case PROTO_RET_DEVICE_BUSY:
      return "device is busy";
    case PROTO_RET_INVALID_MTU:
      return "invalid MTU size";
    default:
      return "?";
  }
}
