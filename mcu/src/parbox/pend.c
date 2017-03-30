#include "types.h"
#include "pend.h"
#include "proto_low.h"
#include "proto_shared.h"

static u08 ack_raised;
static u08 channel_pending[PROTO_MAX_CHANNEL];
u16 pend_total;
u16 pend_mask;

void pend_init(void)
{
  pend_mask = 0;
  pend_total = 0;
  ack_raised = 0;
  for(u08 i=0;i<PROTO_MAX_CHANNEL;i++) {
      channel_pending[i] = 0;
  }
}

void pend_handle(void)
{
  /* if ack was raised then disable it */
  if(ack_raised) {
    proto_low_ack_hi(); /* hi=inactive */
    ack_raised = 0;
  }
}

u08 pend_add_req(u08 channel)
{
  if(channel > PROTO_MAX_CHANNEL) {
    return PEND_RET_INVALID;
  }
  u08 *cp = &channel_pending[channel];
  if(*cp == 0) {
    /* no request on this channel pending yet. set mask */
    pend_mask |= 1 << channel;
  }
  else if(*cp == 255) {
    /* ignore request if too many are already pending */
    return PEND_RET_TOO_MANY;
  }
  /* account request */
  (*cp)++;
  pend_total++;
  return PEND_RET_OK;
}

u08 pend_rem_req(u08 channel)
{
  if(channel > PROTO_MAX_CHANNEL) {
    return PEND_RET_INVALID;
  }
  u08 *cp = &channel_pending[channel];
  if(*cp == 0) {
    return PEND_RET_INVALID;
  }
  else if(*cp == 1) {
    /* clear mask again */
    pend_mask &= ~(1 << channel);
  }
  (*cp)--;
  pend_total--;
  return PEND_RET_OK;
}

u08 pend_clear_reqs(u08 channel)
{
  if(channel > PROTO_MAX_CHANNEL) {
    return PEND_RET_INVALID;
  }
  u08 *cp = &channel_pending[channel];
  u08 num = *cp;
  if(num > 0) {
    /* clear mask again */
    pend_mask &= ~(1 << channel);
  }
  *cp = 0;
  pend_total -= num;
  return PEND_RET_OK;
}
