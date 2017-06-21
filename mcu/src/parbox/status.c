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
static u08 old_state;

void status_init(void)
{
  attached = 0;
  error_code = STATUS_NO_ERROR;
  pending_channel = STATUS_NO_CHANNEL;
  irq_triggered = 0;
  old_state = 0;
}

void status_handle(void)
{
  // reset irq if it was set
  if(irq_triggered) {
    irq_triggered = 0;
    proto_low_ack_hi();
    DS("I-"); DNL;
  }
}

void status_update(void)
{
  // if an error is set then show it always (suppress pending if necessary)
  u08 bits = 0;
  if(attached) {
    bits |= PROTO_STATUS_ATTACHED;
  }
  if(error_code != 0) {
    bits |= PROTO_STATUS_ERROR;
  }
  // if a channel is pending
  else if(pending_channel != STATUS_NO_CHANNEL) {
    bits = pending_channel << 4 | PROTO_STATUS_READ_PENDING;
  }

  // set bits
  if(bits != old_state) {
    old_state = bits;
    DS("s:"); DB(bits); DNL;
    u08 done = proto_low_set_status(bits);
    if(done) {
      DS("s."); DNL;
    } else {
      DS("s!"); DNL;
    }
  } else {
    DS("s?"); DB(bits); DNL;
  }
}

u08 status_get_current(void)
{
  DS("sr:"); DB(old_state); DNL;
  return old_state;
}

void status_set_error(u08 error)
{
  DS("e+"); DB(error); DNL;
  error_code = error;
  status_update();
}

u08 status_clear_error(void)
{
  DS("e-"); DB(error_code); DNL;
  u08 e = error_code;
  error_code = STATUS_NO_ERROR;
  status_update();
  return e;
}

void status_attach(void)
{
  if(!attached) {
    DS("sa"); DNL;
    attached = 1;
  } else {
    DS("sa?"); DNL;
    status_set_error(PROTO_ERROR_ALREADY_ATTACHED);
  }
  status_update();
}

void status_detach(void)
{
  if(attached) {
    DS("sd"); DNL;
    attached = 0;
  } else {
    DS("sd?"); DNL;
    status_set_error(PROTO_ERROR_ALREADY_DETACHED);
  }
  status_update();
}

void status_set_pending(u08 channel)
{
  // no channel pending yet -> trigger ack irq
  DS("p+"); DB(channel); DNL;
  if(pending_channel == STATUS_NO_CHANNEL) {
    DS("I+"); DNL;
    proto_low_ack_lo();
    irq_triggered = 1;
  }
  pending_channel = channel;
  status_update();
}

void status_clear_pending(void)
{
  DS("p-"); DNL;
  pending_channel = STATUS_NO_CHANNEL;
  status_update();
}

u08 status_is_pending(void)
{
  return pending_channel != STATUS_NO_CHANNEL;
}

void proto_api_read_is_pending(u08 cmd) __attribute__ ((weak, alias("status_is_pending")));
u08 proto_api_get_end_status(void) __attribute__ ((weak, alias("status_get_current")));
