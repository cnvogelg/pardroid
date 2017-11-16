#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_STATUS

#include "debug.h"
#include "system.h"

#include "status.h"
#include "proto_low.h"
#include "proto_shared.h"
#include "proto.h"

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
  u08 bits = status_get_current();

  // set bits
  if(bits != old_state) {
    u08 cmd = proto_current_cmd();
    if(cmd == 0xff) {
      old_state = bits;
      DS("sw:"); DB(bits);
      u08 done = proto_low_set_status(bits);
      if(done) {
        DS("."); DNL;
      } else {
        DS("?"); DNL;
      }
    } else {
      DS("s-"); DNL;
    }
  }
}

u08 status_get_current(void)
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
  DS("sr:"); DB(bits); DNL;
  return bits;
}

void status_set_error(u08 code)
{
  error_code = code;
  DS("e:"); DB(error_code); DNL;
  status_update();
}

void status_set_error_mask(u08 mask)
{
  error_code |= mask;
  DS("e+"); DB(error_code); DNL;
  status_update();
}

void status_clear_error_mask(u08 mask)
{
  error_code &= ~mask;
  DS("e-"); DB(error_code); DNL;
  status_update();
}

u08 status_clear_error(void)
{
  DS("e:"); DB(error_code); DNL;
  u08 error = error_code;
  error_code = 0;
  status_update();
  return error;
}

void status_attach(void)
{
  if(!attached) {
    DS("sa"); DNL;
    attached = 1;
  } else {
    DS("sa?"); DNL;
    status_set_error_mask(PROTO_ERROR_ALREADY_ATTACHED);
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
    status_set_error_mask(PROTO_ERROR_ALREADY_DETACHED);
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

u08 proto_api_get_end_status(void) __attribute__ ((weak, alias("status_get_current")));
