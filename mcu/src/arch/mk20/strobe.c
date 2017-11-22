#include <stdint.h>
#include "kinetis.h"

#include "autoconf.h"
#include "types.h"
#include "strobe.h"
#include "knok.h"

void strobe_init(void)
{
}

void strobe_exit(void)
{
}

u08 strobe_get_key(u32 *key)
{
  *key = KNOK_KEY_BOOT;
  return 1;
}
