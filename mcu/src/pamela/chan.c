#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "chan.h"
#include "proto.h"

static u16 rx_pend;
static u16 error;

u16 chan_getclr_rx_pending(void)
{
  u16 v = rx_pend;
  rx_pend = 0;
  return v;
}

u16 chan_getclr_error(void)
{
  u16 v = error;
  error = 0;
  return v;
}

void chan_set_rx_pending(u16 mask)
{
  u16 new = rx_pend | mask;
  if(rx_pend != new) {
    rx_pend = new;
    proto_trigger_signal();
  }
}

void chan_set_error(u16 mask)
{
  u16 new = error | mask;
  if(error != new) {
    error = new;
    proto_trigger_signal();
  }
}
