#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "proto.h"
#include "proto_low.h"

/* proto signals */
#define clk_mask    pout_mask
#define rak_mask    busy_mask
#define pend_mask   sel_mask

#define DDR_DATA_OUT  0xff
#define DDR_DATA_IN   0x00

struct proto_handle {
    struct pario_port   *port;
    struct timer_handle *timer;
    ULONG                timeout_s;
    ULONG                timeout_ms;
};

struct proto_handle *proto_init(struct pario_port *port, struct timer_handle *th)
{
  struct proto_handle *ph;

  ph = AllocMem(sizeof(struct proto_handle), MEMF_CLEAR);
  if(ph == NULL) {
    return NULL;
  }
  ph->port = port;
  ph->timer = th;
  ph->timeout_s  = 0;
  ph->timeout_ms = 500000UL;

  /* control: clk=out(1) rak,pend=in*/
  *port->ctrl_ddr |= port->clk_mask;
  *port->ctrl_ddr &= ~(port->rak_mask | port->pend_mask);
  *port->ctrl_port |= port->all_mask;

  /* data: port=0, ddr=0xff (OUT) */
  *port->data_port = PROTO_CMD_IDLE;
  *port->data_ddr  = 0xff;

  return ph;
}

void proto_exit(struct proto_handle *ph)
{
  if(ph == NULL) {
    return;
  }
  /* free handle */
  FreeMem(ph, sizeof(struct proto_handle));
}

int proto_ping(struct proto_handle *ph)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_no_value(port, timeout_flag, PROTO_CMD_PING);
  timer_stop(ph->timer);

  return result;
}

int proto_reset(struct proto_handle *ph)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_no_value(port, timeout_flag, PROTO_CMD_RESET);
  timer_stop(ph->timer);

  return result;
}

int proto_reg_read(struct proto_handle *ph, UBYTE reg, UWORD *data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  UBYTE cmd = reg + PROTO_CMD_REG_READ;
  if(reg >= NUM_REG) {
    return PROTO_RET_INVALID_REG;
  }

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_read_word(port, timeout_flag, cmd, (UBYTE *)data);
  timer_stop(ph->timer);

  return result;
}

int proto_reg_write(struct proto_handle *ph, UBYTE reg, UWORD *data)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  UBYTE cmd = reg + PROTO_CMD_REG_WRITE;
  if(reg >= NUM_REG) {
    return PROTO_RET_INVALID_REG;
  }

  timer_start(ph->timer, ph->timeout_s, ph->timeout_ms);
  int result = proto_low_write_word(port, timeout_flag, cmd, (UBYTE *)data);
  timer_stop(ph->timer);

  return result;
}

int proto_msg_write(struct proto_handle *ph, UBYTE chn, ULONG *msgiov)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  UBYTE cmd = chn + PROTO_CMD_MSG_WRITE;
  if(chn >= NUM_CHANNEL) {
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

int proto_msg_read(struct proto_handle *ph, UBYTE chn, ULONG *msgiov)
{
  struct pario_port *port = ph->port;
  volatile BYTE *timeout_flag = timer_get_flag(ph->timer);
  UBYTE cmd = chn + PROTO_CMD_MSG_READ;
  if(chn >= NUM_CHANNEL) {
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

int proto_msg_write_single(struct proto_handle *ph, UBYTE chn, UBYTE *buf, ULONG num_words)
{
  ULONG msgiov[] = {
    num_words, /* total size */
    num_words, /* chunk size */
    (ULONG)buf,/* chunk pointer */
    0          /* last chunk */
  };
  return proto_msg_write(ph, chn, msgiov);
}

int proto_msg_read_single(struct proto_handle *ph, UBYTE chn, UBYTE *buf, ULONG *max_words)
{
  ULONG msgiov[] = {
    *max_words, /* total size */
    *max_words, /* chunk size */
    (ULONG)buf,/* chunk pointer */
    0          /* last chunk */
  };
  int result = proto_msg_read(ph, chn, msgiov);
  /* store returned result size */
  *max_words = msgiov[0];
  return result;
}

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
    case PROTO_RET_INVALID_REG:
      return "invalid register";
    case PROTO_RET_INVALID_CHANNEL:
      return "invalid channel";
    case PROTO_RET_MSG_TOO_LARGE:
      return "message too large";
    default:
      return "unknown";
  }
}
