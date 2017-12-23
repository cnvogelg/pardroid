#include <stdint.h>
#include "kinetis.h"
#include "arch.h"

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

void strobe_send_begin(rom_pchar data, u16 size)
{
}

void strobe_send_end(void)
{
}

u08  strobe_read_flag(void)
{
  return 0;
}

void strobe_pulse_ack(void)
{
}
