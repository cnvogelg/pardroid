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
static u16 rx_pend;
static u16 error;

void status_set_busy(void)
{
  DS("bsy+"); DNL;
  if(busy == 0) {
    proto_low_busy_hi();
    proto_trigger_signal();
  }
  busy++;
}

void status_clr_busy(void)
{
  DS("bsy-"); DNL;
  busy--;
  if(busy == 0) {
    proto_low_busy_lo();
    proto_trigger_signal();
  }
}

u08 status_is_busy(void)
{
  return busy > 0;
}

void status_set_rx_pending(u08 chan)
{
  u16 mask = rx_pend | (1 << chan);
  status_set_rx_pending_mask(mask);
}

void status_clr_rx_pending(u08 chan)
{
  u16 mask = rx_pend & ~(1 << chan);
  status_set_rx_pending_mask(mask);
}

void status_set_error(u08 chan)
{
  u16 mask = error | (1 << chan);
  status_set_error_mask(mask);
}

void status_clr_error(u08 chan)
{
  u16 mask = error & ~(1 << chan);
  status_set_error_mask(mask);
}

void status_set_rx_pending_mask(u16 mask)
{
  if(mask != 0) {
    proto_trigger_signal();
  }
  rx_pend = mask;
}

void status_set_error_mask(u16 mask)
{
  if(mask != 0) {
    proto_trigger_signal();
  }
  error = mask;
}

void status_set_mask(u32 mask)
{
  if(mask != 0) {
    proto_trigger_signal();
  }
  rx_pend = (u16)(mask & 0xffff);
  error = (u16)(mask >> 16);
}

u32 status_get_mask()
{
  u32 mask = rx_pend | ((u32)error << 16);
  if(busy > 0) {
    mask |= PROTO_STATUS_MASK_BUSY;
  }
  return mask;
}
