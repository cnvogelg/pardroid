#ifndef CHANNEL_H
#define CHANNEL_H

#include <exec/lists.h>
#include <exec/semaphores.h>

#include "request.h"

struct channel {
  UBYTE        id;
  UBYTE        pad0;
  struct List  read_requests;
  struct SignalSemaphore  sem;
};
typedef struct channel channel_t;

extern channel_t *channel_create(UBYTE id);
extern void channel_delete(channel_t *chn);

extern void channel_add_read_request(channel_t *chn, request_t *req);
extern request_t *channel_get_next_read_request(channel_t *chn);

#endif