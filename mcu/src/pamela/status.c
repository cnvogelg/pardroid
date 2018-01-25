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
static u08 event_mask;
static u08 pending_mask;
static u08 irq_flags;
static u08 old_state;
static u08 pending_channel;

void status_init(void)
{
  attached = 0;
  event_mask = 0;
  pending_mask = 0;
  irq_flags = IRQ_FLAG_NONE;
  old_state = 0;
  pending_channel = 7;
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
    DS("su:"); DB(bits); DC('<'); DB(old_state);
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

    // issue irq if event or pending was set
    u08 changed = bits ^ old_state;
    if((changed & PROTO_STATUS_READ_PENDING) && (bits & PROTO_STATUS_READ_PENDING)) {
      irq_flags |= IRQ_FLAG_REQUEST;
    }
    if((changed & PROTO_STATUS_EVENTS) && (bits & PROTO_STATUS_EVENTS)) {
      irq_flags |= IRQ_FLAG_REQUEST;
    }

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
  if(event_mask != 0) {
    bits |= PROTO_STATUS_EVENTS;
  }
  // if a channel is pending
  else if(pending_mask != 0) {
    bits = pending_channel << 4 | PROTO_STATUS_READ_PENDING;
  }
  DS("sr:"); DB(bits); DNL;
  return bits;
}

// ----- events -----

void status_clear_events(void)
{
  event_mask = 0;
  DS("e0"); DNL;
  status_update();
}

void status_set_events(u08 evmsk)
{
  event_mask = evmsk;
  DS("e="); DB(event_mask); DNL;
  status_update();
}

void status_set_event_mask(u08 mask)
{
  event_mask |= mask;
  DS("e+"); DB(event_mask); DNL;
  status_update();
}

void status_clear_event_mask(u08 mask)
{
  event_mask &= ~mask;
  DS("e-"); DB(event_mask); DNL;
  status_update();
}

u08 status_get_event_mask(void)
{
  return event_mask;
}

// ----- attach/detach -----

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

// ----- read pending -----

static u08 find_next_channel(u08 mask, u08 chn)
{
  for(u08 i=0;i<8;i++) {
    chn = (chn + 1) & 7;
    u08 bmask = 1 << chn;
    if(mask & bmask) {
      return chn;
    }
  }
  return 0xff;
}

static void pending_update(u08 new_mask)
{
  // nothing has changed
  if(new_mask == pending_mask) {
    DS("u="); DNL;
    return;
  }

  u08 do_update = 0;

  // old mask was zero. find next channel
  if(pending_mask == 0) {
    // new mask has at least one bit set
    pending_channel = find_next_channel(new_mask, pending_channel);
    DS("N="); DB(pending_channel);
    do_update = 1;
  }
  // pending mask was set
  else {
    // if current pending channel was cleared then find next
    u08 pc_mask = 1 << pending_channel;
    if((pc_mask & new_mask) == 0) {
      // any remaining pending bits set?
      u08 rem_mask = new_mask & ~pc_mask;
      if(rem_mask != 0) {
        pending_channel = find_next_channel(new_mask, pending_channel);
        DS("n="); DB(pending_channel);
      } else {
        // no pending anymore
        DC('z');
      }
      do_update = 1;
    }
    // if current pending channel is still pending then keep it
    // and do not update status
    else {
      DC('k');
    }
  }

  // done
  pending_mask = new_mask;
  if(do_update) {
    status_update();
  }

  DNL;
}

void status_set_pending(u08 pmask)
{
  // no channel pending yet -> trigger ack irq
  DS("p="); DB(pmask);
  pending_update(pmask);
}

void status_clear_pending(void)
{
  DS("p0");
  pending_update(0);
}

void status_reset_pending(void)
{
  DS("p0r");
  pending_update(0);
  pending_channel = 7;
}

u08 status_get_pending_mask(void)
{
  return pending_mask;
}

void status_set_pending_mask(u08 mask)
{
  DS("p+"); DB(mask);
  pending_update(pending_mask | mask);
}

void status_clear_pending_mask(u08 mask)
{
  DS("p-"); DB(mask);
  pending_update(pending_mask & ~mask);
}

u08 proto_api_get_end_status(void) __attribute__ ((weak, alias("status_get_current")));
