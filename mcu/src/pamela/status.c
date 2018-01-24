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

#define IRQ_FLAG_NONE       0
#define IRQ_FLAG_REQUEST    1
#define IRQ_FLAG_ACTIVE     2

static u08 attached;
static u08 events;
static u08 pending_channel;
static u08 irq_flags;
static u08 old_state;

void status_init(void)
{
  attached = 0;
  events = STATUS_NO_EVENTS;
  pending_channel = STATUS_NO_CHANNEL;
  irq_flags = IRQ_FLAG_NONE;
  old_state = 0;
}

void status_handle(void)
{
  // enable ack irq
  if(irq_flags & IRQ_FLAG_REQUEST)
  {
    irq_flags = IRQ_FLAG_ACTIVE;
    proto_low_ack_lo();
    DS("I+"); DNL;
  }
  // reset irq if it was set
  else if(irq_flags & IRQ_FLAG_ACTIVE) {
    irq_flags &= ~IRQ_FLAG_ACTIVE;
    proto_low_ack_hi();
    DS("I-"); DNL;
  }
}

void status_update(void)
{
  u08 bits = status_get_current();

  // set bits
  if(bits != old_state) {
    DS("su:"); DB(bits);
    u08 cmd = proto_current_cmd();
    if(cmd == 0xff) {
      // no command running -> update state now
      u08 done = proto_low_set_status(bits);
      if(done) {
        DS("."); DNL;
      } else {
        DS("?"); DNL;
      }
    }
    irq_flags |= IRQ_FLAG_REQUEST;
    old_state = bits;
  } else {
    DS("su="); DB(bits); DNL;
  }
}

u08 status_get_current(void)
{
  // if an error is set then show it always (suppress pending if necessary)
  u08 bits = 0;
  if(attached) {
    bits |= PROTO_STATUS_ATTACHED;
  }
  if(events != 0) {
    bits |= PROTO_STATUS_EVENTS;
  }
  // if a channel is pending
  else if(pending_channel != STATUS_NO_CHANNEL) {
    bits = pending_channel << 4 | PROTO_STATUS_READ_PENDING;
  }
  DS("sr:"); DB(bits); DNL;
  return bits;
}

void status_clear_events(void)
{
  events = 0;
  DS("e=0"); DNL;
  status_update();
}

void status_set_events(u08 evmsk)
{
  events = evmsk;
  DS("e:"); DB(events); DNL;
  status_update();
}

void status_set_event_mask(u08 mask)
{
  events |= mask;
  DS("e+"); DB(events); DNL;
  status_update();
}

void status_clear_event_mask(u08 mask)
{
  events &= ~mask;
  DS("e-"); DB(events); DNL;
  status_update();
}

u08 status_get_events(void)
{
  DS("e:"); DB(events); DNL;
  return events;
}

void status_attach(void)
{
  if(!attached) {
    DS("sa"); DNL;
    attached = 1;
  } else {
    DS("sa?"); DNL;
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
  }
  status_update();
}

void status_set_pending(u08 channel)
{
  // no channel pending yet -> trigger ack irq
  DS("p+"); DB(channel); DNL;
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
