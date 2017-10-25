#include "types.h"
#include "arch.h"
#include "debug.h"
#include "system.h"

#include "proto_shared.h"
#include "offset.h"

#ifndef NUM_SLOTS
#define NUM_SLOTS PROTO_MAX_CHANNEL
#endif

static u32 offset[NUM_SLOTS];

void offset_set(u08 slot, u32 val)
{
  if(slot < NUM_SLOTS) {
    offset[slot] = val;
  }
}

u32 offset_get(u08 slot) {
  if(slot < NUM_SLOTS) {
    return offset[slot];
  } else {
    return 0;
  }
}

void func_api_set_offset(u08 num, u32 val) __attribute__ ((weak, alias("offset_set")));
u32 func_api_get_offset(u08 num) __attribute__ ((weak, alias("offset_get")));
