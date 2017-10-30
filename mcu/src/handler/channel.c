#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_CHANNEL

#include "types.h"
#include "proto_shared.h"
#include "arch.h"
#include "handler.h"
#include "reg.h"
#include "reg_def.h"
#include "debug.h"
#include "channel.h"

#define NUM_CHANNELS PROTO_MAX_CHANNEL

#define FLAG_NONE          0
#define FLAG_HANDLER       1
#define FLAG_INIT          2
#define FLAG_OPEN          4

#define CONTROL_OPEN       0
#define CONTROL_CLOSE      1

struct channel {
  u08 flags;
  u16 mtu;
};
typedef struct channel channel_t;

static channel_t channels[NUM_CHANNELS];
static u08 chn_idx;

REG_TABLE_BEGIN(channel)
  REG_TABLE_RW_FUNC(channel_reg_index),
  REG_TABLE_RW_FUNC(channel_reg_ctrl_status),
  REG_TABLE_RW_FUNC(channel_reg_mtu)
REG_TABLE_END(channel, PROTO_REGOFFSET_CHANNEL, REG_TABLE_REF(base))

void channel_init(void)
{
  /* init channel state */
  u08 num_handler = HANDLER_GET_TABLE_SIZE();
  DS("Ci:"); DB(num_handler); DC(','); DB(NUM_CHANNELS); DNL;
  for(u08 i=0; i<NUM_CHANNELS;i++) {
    channel_t *c = &channels[i];
    if(i < num_handler) {
      c->flags = FLAG_HANDLER;
      /* try to init handler */
      u08 status = handler_init(i);
      DS("Cn:"); DB(i); DC('='); DB(status);
      if(status == HANDLER_OK) {
        c->flags |= FLAG_INIT;
        /* setup mtu */
        u16 min, max;
        handler_get_mtu(i, &max, &min);
        c->mtu = max;
        DC(','); DW(max);
      }
      DNL;
    } else {
      c->flags = FLAG_NONE;
    }
  }
  /* reset index */
  chn_idx = 0;
}

void channel_work(void)
{
  for(u08 i=0;i<NUM_CHANNELS;i++) {
    channel_t *c = &channels[i];
    u08 flags = c->flags;
    /* trigger work for initialized handlers */
    if(flags & FLAG_OPEN) {
      handler_work(i, flags);
    }
  }
}

/* ----- access channel ----- */

u16 channel_get_mtu(u08 chn)
{
  if(chn < NUM_CHANNELS) {
    return channels[chn].mtu;
  } else {
    return 0;
  }
}

u08 channel_get_flags(u08 chn)
{
  if(chn < NUM_CHANNELS) {
    return channels[chn].flags;
  } else {
    return 0;
  }
}

/* ----- control funcs ----- */

/* open current channel */
static void open(void)
{
  channel_t *c = &channels[chn_idx];
  u08 flags = c->flags;
  /* not yet open? */
  if((flags & FLAG_OPEN) == 0) {
    /* ask handler to open channel */
    u08 status = handler_open(chn_idx);
    if(status == HANDLER_OK) {
      /* ok. we are open */
      c->flags |= FLAG_OPEN;
    }
  }
}

/* close current channel */
static void close(void)
{
  channel_t *c = &channels[chn_idx];
  u08 flags = c->flags;
  /* can close? */
  if((flags & FLAG_OPEN) == FLAG_OPEN) {
    handler_close(chn_idx);
    c->flags &= ~FLAG_OPEN;
  }
}

/* ----- register funcs ----- */

/* set current channel index */
void channel_reg_index(u16 *v,u08 mode)
{
  if(mode == REG_MODE_READ) {
    *v = chn_idx;
  } else {
    u08 val = (u08)*v;
    if(val < NUM_CHANNELS) {
      chn_idx = val;
    }
  }
}

/* control/status register */
void channel_reg_ctrl_status(u16 *v,u08 mode)
{
  if(mode == REG_MODE_READ) {
    /* read: status */
    *v = channels[chn_idx].flags;
  } else {
    /* write: control commands */
    u08 cmd = (u08)*v;
    switch(cmd) {
      case CONTROL_OPEN:
        open();
        break;
      case CONTROL_CLOSE:
        close();
        break;
    }
  }
}

/* get/set current channel MTU */
void channel_reg_mtu(u16 *v,u08 mode)
{
  if(mode == REG_MODE_READ) {
    *v = channels[chn_idx].mtu;
  } else {
    u16 mtu_min;
    u16 mtu_max;
    handler_get_mtu(chn_idx, &mtu_max, &mtu_min);
    u16 val = *v;
    if((val >= mtu_min) && (val <= mtu_max)) {
      channels[chn_idx].mtu = val;
    }
  }
}

