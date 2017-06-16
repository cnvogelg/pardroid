#include "types.h"
#include "arch.h"
#include "debug.h"
#include "system.h"

#include "status.h"
#include "proto_low.h"
#include "proto_shared.h"

static u08 attached;
static u08 error_code;
static u08 pending_channel;
static u08 irq_triggered;

void status_init(void)
{
  attached = 0;
  error_code = STATUS_NO_ERROR;
  pending_channel = STATUS_NO_CHANNEL;
  irq_triggered = 0;
}

void status_handle(void)
{
  // reset irq if it was set
  if(irq_triggered) {
    irq_triggered = 0;
    proto_low_ack_hi();
  }
}

void status_update(void)
{
  // if an error is set then show it always (suppress pending if necessary)
  if(error_code != 0) {
    u08 bits = PROTO_STATUS_ERROR;
    if(attached) {
      bits |= PROTO_STATUS_ATTACHED;
    }
    proto_low_pend_hi();
    proto_low_set_status(bits);
  }
  // if a channel is pending
  else if(pending_channel != STATUS_NO_CHANNEL) {
    u08 bits = pending_channel << 5;
    proto_low_pend_lo();
    proto_low_set_status(bits);
  }
  // regular bits (no pending channel)
  else {
    u08 bits = 0;
    if(attached) {
      bits |= PROTO_STATUS_ATTACHED;
    }
    proto_low_pend_hi();
    proto_low_set_status(bits);
  }
}

void status_set_error(u08 error)
{
  error_code = error;
}

u08 status_clear_error(void)
{
  u08 e = error_code;
  error_code = STATUS_NO_ERROR;
  return e;
}

void status_attach(void)
{
  if(!attached) {
    attached = 1;
  } else {
    status_set_error(PROTO_ERROR_ALREADY_ATTACHED);
  }
}

void status_detach(void)
{
  if(attached) {
    attached = 0;
  } else {
    status_set_error(PROTO_ERROR_ALREADY_DETACHED);
  }
}

void status_set_pending(u08 channel)
{
  // no channel pending yet -> trigger ack irq
  if(pending_channel == STATUS_NO_CHANNEL) {
    proto_low_ack_lo();
    irq_triggered = 1;
  }
  pending_channel = channel;
}

void status_clear_pending(void)
{
  pending_channel = STATUS_NO_CHANNEL;
}

u08 status_is_pending(void)
{
  return pending_channel != STATUS_NO_CHANNEL;
}

void proto_api_read_is_pending(u08 cmd) __attribute__ ((weak, alias("status_is_pending")));
