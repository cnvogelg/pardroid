#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_OFFSET

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
    DS("Ow:"); DL(val); DNL;
    offset[slot] = val;
  }
}

u32 offset_get(u08 slot) {
  if(slot < NUM_SLOTS) {
    u32 val = offset[slot];
    DS("Or:"); DL(val); DNL;
    return val;
  } else {
    return 0;
  }
}

void func_api_set_offset(u08 num, u32 val) __attribute__ ((weak, alias("offset_set")));
u32 func_api_get_offset(u08 num) __attribute__ ((weak, alias("offset_get")));
