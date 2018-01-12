#include "types.h"
#include "arch.h"
#include "autoconf.h"

#include "handler.h"
#include "handler_list.h"

// handler
HANDLER_TABLE_BEGIN
  HANDLER_TABLE_ENTRY(null),
  HANDLER_TABLE_ENTRY(echo)
HANDLER_TABLE_END
