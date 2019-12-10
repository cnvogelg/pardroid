#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG CONFIG_DEBUG_PROTO

#include "debug.h"
#include "status.h"
#include "proto_shared.h"
#include "proto_low.h"
#include "proto.h"

static u08 busy;
static u16 status_mask;
static u16 error_mask;

void status_set_busy(void)
{
  DS("bsy+"); DNL;
  if(busy == 0) {
    proto_low_busy_hi();
    status_mask |= PROTO_STATUS_MASK_BUSY;
  }
  busy++;
}

void status_clr_busy(void)
{
  DS("bsy-"); DNL;
  busy--;
  if(busy == 0) {
    proto_low_busy_lo();
    status_mask &= ~PROTO_STATUS_MASK_BUSY;
  }
}

u08 status_is_busy(void)
{
  return busy > 0;
}

void status_set_rx_flag(u08 chan)
{
  status_mask |= (1 << chan);
  proto_trigger_signal();
}

void status_clr_rx_flag(u08 chan)
{
  status_mask &= ~(1 << chan);
}

void status_set_error_flag(u08 chan)
{
  status_mask |= PROTO_STATUS_MASK_ERROR;
  proto_trigger_signal();
}

void status_clr_error_flag(u08 chan)
{
  status_mask &= ~PROTO_STATUS_MASK_ERROR;
}

void status_set_status_mask(u16 mask)
{
  if(mask != 0) {
    proto_trigger_signal();
  }
  status_mask = mask;
}

void status_set_error_mask(u16 mask)
{
  error_mask = mask;
}

u16 status_get_status_mask()
{
  return status_mask;
}

u16 status_get_error_mask()
{
  return error_mask;
}
