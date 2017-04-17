#include "types.h"
#include "pend.h"
#include "proto_low.h"
#include "proto_shared.h"

static u08 ack_raised;
u16 pend_total;

void pend_init(void)
{
  pend_total = 0;
  ack_raised = 0;
}

void pend_handle(void)
{
  /* if ack was raised then disable it */
  if(ack_raised) {
    proto_low_ack_hi(); /* hi=inactive */
    ack_raised = 0;
  }
}

u08 pend_add_req(void)
{
  /* too many pending? */
  if(pend_total == 255) {
    return PEND_RET_TOO_MANY;
  }
  /* if no pending then trigger ack */
  if(pend_total == 0) {
    proto_low_ack_lo();
    ack_raised = 1;
  }
  /* account request */
  pend_total++;
  return PEND_RET_OK;
}

u08 pend_rem_req(void)
{
  if(pend_total == 0) {
    return PEND_RET_INVALID;
  }
  pend_total--;
  return PEND_RET_OK;
}

u08 pend_clear_reqs(void)
{
  pend_total = 0;
  return PEND_RET_OK;
}
