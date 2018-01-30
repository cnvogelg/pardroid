#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_STATUS

#include "debug.h"
#include "system.h"
#include "timer.h"

#include "status.h"
#include "proto_low.h"
#include "proto_shared.h"
#include "proto.h"

#define FLAG_NONE           0
#define FLAG_IRQ_REQUEST    1
#define FLAG_IRQ_ACTIVE     2
#define FLAG_DETACHED       4
#define FLAG_PEND_IRQ       8
#define FLAG_EVENT_IRQ      16
#define FLAG_BUSY           32
#define FLAG_UPDATE         64

static u08 flags;
static u08 event_mask;
static u08 pending_mask;
static u08 old_bits;
static u08 pending_channel;

void status_init(void)
{
  event_mask = 0;
  pending_mask = 0;
  // request an irq right after reset
  // (to report detached state)
  flags = FLAG_DETACHED | FLAG_IRQ_REQUEST;
  old_bits = PROTO_STATUS_DETACHED;
  pending_channel = 7;
}

static u08 status_get_bits(void)
{
  u08 bits = 0;
  // highest prio: detached
  if(flags & FLAG_DETACHED) {
    bits = PROTO_STATUS_DETACHED;
  }
  // then: busy
  else if(flags & FLAG_BUSY) {
    bits = PROTO_STATUS_BUSY;
  }
  // then: event
  else if(event_mask != 0) {
    bits = PROTO_STATUS_EVENTS;
  }
  // finally: pending
  else if(pending_mask != 0) {
    bits = pending_channel << 4 | PROTO_STATUS_READ_PENDING;
  }
  return bits;
}

void status_handle(void)
{
  // need to update state?
  if(flags & FLAG_UPDATE) {
    u08 bits = status_get_bits();
    if(bits != old_bits) {
      DS("su:"); DB(bits); DC('<'); DB(old_bits);
      // try to update
      u08 done = proto_low_set_status(bits);
      if(done) {
        DC('.');
        flags &= ~FLAG_UPDATE;
        old_bits = bits;
      } else {
        DC('?');
      }
      DNL;
    } else {
      DS("su?"); DB(bits); DNL;
      flags &= ~FLAG_UPDATE;
    }
  }

  // only if update was successful then trigger irgs
  // enable ack irq
  if((flags & FLAG_IRQ_REQUEST) && ((flags & FLAG_UPDATE)==0))
  {
    flags &= ~FLAG_IRQ_REQUEST;
    flags |=  FLAG_IRQ_ACTIVE;
    proto_low_ack_lo();
    DS("I+"); DNL;
  }
  // reset irq if it was set
  else if(flags & FLAG_IRQ_ACTIVE) {
    flags &= ~FLAG_IRQ_ACTIVE;
    proto_low_ack_hi();
    DS("I-"); DNL;
  }
}

// called after a command to update status bits
u08 status_after_cmd(void)
{
  u08 bits = status_get_bits();
  // no more update required
  flags &= ~FLAG_UPDATE;
  old_bits = bits;
  DS("Sc:"); DB(bits); DNL;
  return bits;
}

static void status_update(void)
{
  u08 bits = status_get_bits();

  // if any event is active and changed
  if((bits != 0) && (bits != old_bits)) {
    DS("Ic:"); DB(bits); DC('<'); DB(old_bits);
    flags |= FLAG_IRQ_REQUEST;
  }

  // status event
  if(bits & PROTO_STATUS_EVENTS) {
    if(flags & FLAG_EVENT_IRQ) {
      flags &= ~FLAG_EVENT_IRQ;
      DS("Ie");
      flags |= FLAG_IRQ_REQUEST;
    }
  }
  // pending event
  else if(bits & PROTO_STATUS_READ_PENDING) {
    if(flags & FLAG_PEND_IRQ) {
      flags &= ~FLAG_PEND_IRQ;
      DS("Ip");
      flags |= FLAG_IRQ_REQUEST;
    }
  }
  DNL;

  // request update in handler
  flags |= FLAG_UPDATE;
}

// ----- events -----

void status_clear_events(void)
{
  event_mask = 0;
  DS("e0"); DNL;
  status_update();
}

u08 status_get_event_mask(void)
{
  return event_mask;
}

void status_set_event(u08 chn)
{
  if(chn >= PROTO_MAX_CHANNEL) {
    return;
  }
  event_mask |= 1 << chn;
  DS("e+"); DB(event_mask); DNL;
  flags |= FLAG_EVENT_IRQ;
  status_update();
}

// ----- attach/detach -----

void status_attach(void)
{
  if(flags & FLAG_DETACHED) {
    DS("sa"); DNL;
    flags &= ~FLAG_DETACHED;
  } else {
    DS("sa?"); DNL;
  }
  status_update();
}

void status_detach(void)
{
  if((flags & FLAG_DETACHED)==0) {
    DS("sd"); DNL;
    flags |= FLAG_DETACHED;
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

void status_set_pending(u08 chn)
{
  // no channel pending yet -> trigger ack irq
  DS("p+"); DB(chn);

  u08 cmask = 1 << chn;
  u08 irq = 0;

  // channel already set? refresh!
  if((cmask & pending_mask) == cmask) {
    // if we are the only pending one then keep it
    if(pending_mask == cmask) {
      DC('#');
      irq = 1;
    }
    else {
      // ir we are the active pending channel then search another one
      if(pending_channel == chn) {
        pending_channel = find_next_channel(pending_mask, pending_channel);
        DC(':'); DB(pending_channel);
        irq = 1;
      }
      // we are not active. do nothing
      else {
        DC('.');
      }
    }
  }
  // this channel was not set yet
  else {
    // no pending channel set yet -> take mine
    if(pending_mask == 0) {
      DC('!');
      pending_channel = chn;
      irq = 1;
    }
  }

  // set mask
  pending_mask |= cmask;
  DNL;
  if(irq) {
    flags |= FLAG_PEND_IRQ;
  }
  status_update();
}

void status_clear_pending(u08 chn)
{
  DS("p-"); DB(chn);

  u08 cmask = 1 << chn;

  // nothing to clear...
  if((cmask & pending_mask) == 0) {
    DC('?'); DNL;
    return;
  }

  // clear pending bit
  pending_mask &= ~cmask;

  // was my channel the active pending channel?
  if(pending_channel == chn) {
    // need to find a new one?
    if(pending_mask != 0) {
        pending_channel = find_next_channel(pending_mask, pending_channel);
        DC(':'); DB(pending_channel);
        // trigger irq for that one
        flags |= FLAG_PEND_IRQ;
    }
  }
  DNL;
  status_update();
}

// ----- busy -----

// after set_busy the caller won't return to status_handle() for a while
// so we have to update the state and trigger the irq directly here in place
void status_set_busy(void)
{
  // already set busy..
  if(flags & FLAG_BUSY) {
    return;
  }

  flags |= FLAG_BUSY;
  u08 bits = status_get_bits();
  old_bits = bits;

  // wait until we could set the status (or reset on failure :)
  while(1) {
    u08 done = proto_low_set_status(bits);
    if(done) {
      break;
    }
    DC('#');
  }

  // trigger irq
  proto_low_ack_lo();
  timer_delay_1us();
  proto_low_ack_hi();

  DS("B+"); DNL;
}

void status_clear_busy(void)
{
  // busy not set...
  if((flags & FLAG_BUSY) == 0) {
    return;
  }

  flags &= ~FLAG_BUSY;
  u08 bits = status_get_bits();
  old_bits = bits;

  // wait until we could set the status (or reset on failure :)
  while(1) {
    u08 done = proto_low_set_status(bits);
    if(done) {
      break;
    }
    DC('#');
  }

  // trigger irq
  proto_low_ack_lo();
  timer_delay_1us();
  proto_low_ack_hi();

  DS("B-"); DNL;
}

u08 proto_api_get_end_status(void) __attribute__ ((weak, alias("status_after_cmd")));
