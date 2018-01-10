#include "types.h"
#include "arch.h"
#include "system.h"

#define DEBUG CONFIG_DEBUG_HANDLER

#include "debug.h"
#include "channel.h"
#include "proto_shared.h"

#define NUM_CHANNEL   PROTO_MAX_CHANNEL

static u08 channel_map[NUM_CHANNEL];

void channel_init(void)
{
  for(u08 i=0;i<NUM_CHANNEL;i++) {
    channel_map[i] = CHANNEL_INVALID;
  }
}

u08  channel_alloc(u08 id)
{
  for(u08 i=0;i<NUM_CHANNEL;i++) {
    if(channel_map[i] == CHANNEL_INVALID) {
      channel_map[i] = id;
      return i;
    }
  }
  return CHANNEL_INVALID;
}

void channel_free(u08 chn)
{
  channel_map[chn] = CHANNEL_INVALID;
}

u08  channel_get_id(u08 chn)
{
  return channel_map[chn];
}
