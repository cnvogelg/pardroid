#include "types.h"
#include "arch.h"
#include "debug.h"
#include "system.h"

#include "action.h"
#include "func.h"

// ----- actions -----
ACTION_TABLE_BEGIN
  // 0: NOP
  ACTION_TABLE_FUNC_FLAGS(action_nop, ACTION_FLAG_NO_REPLY),
  // 1: PING
  ACTION_TABLE_FUNC(action_ping),
  // 2: BOOTLOADER
  ACTION_TABLE_FUNC_FLAGS(action_bootloader, ACTION_FLAG_NO_REPLY),
  // 3: RESET
  ACTION_TABLE_FUNC_FLAGS(action_reset, ACTION_FLAG_END_BEFORE),
  // 4: ATTACH
  ACTION_TABLE_FUNC(action_attach),
  // 5: DETACH
  ACTION_TABLE_FUNC(action_detach)
ACTION_TABLE_END

// ----- functions -----
FUNC_TABLE_BEGIN
  // --- registers ---
  // 0
  FUNC_TABLE_GET_FUNC(func_regaddr_get),
  // 1
  FUNC_TABLE_SET_FUNC(func_regaddr_set),
  // 2
  FUNC_TABLE_GET_FUNC(func_reg_read),
  // 3
  FUNC_TABLE_SET_FUNC(func_reg_write),
  // --- offsets ---
  // 4
  FUNC_TABLE_GET_FUNC(func_offslot_get),
  // 5
  FUNC_TABLE_SET_FUNC(func_offslot_set),
  // 6
  FUNC_TABLE_GET_FUNC_LONG(func_offset_get),
  // 7
  FUNC_TABLE_SET_FUNC_LONG(func_offset_set),
FUNC_TABLE_END

