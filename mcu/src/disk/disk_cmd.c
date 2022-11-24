#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_DISK

#include "debug.h"

#include "pamela.h"
#include "disk.h"
#include "parbox/ports.h"
#include "disk/wire.h"

#include <stdlib.h>

u08 disk_cmd_handle(u08 *buf, u16 size, u16 *ret_size)
{
  return PAMELA_OK;
}
