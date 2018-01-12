#include "types.h"
#include "arch.h"
#include "autoconf.h"

#include "driver_list.h"

// driver
DRIVER_TABLE_BEGIN
  DRIVER_TABLE_ENTRY(blk_null),
#ifdef CONFIG_DRIVER_ENC28J60
  DRIVER_TABLE_ENTRY(eth_enc),
#endif
#ifdef CONFIG_DRIVER_SDCARD
  DRIVER_TABLE_ENTRY(blk_sdraw),
#endif
DRIVER_TABLE_END
