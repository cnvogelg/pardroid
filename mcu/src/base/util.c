#include "autoconf.h"
#include "types.h"
#include "arch.h"

#include "util.h"

void util_put_uword(u16 val, u08 *buf)
{
  buf[0] = (u08)(val & 0xff);
  buf[1] = (u08)(val >> 8);
}

u16  util_get_uword(u08 *buf)
{
  return buf[0] | (buf[1] << 8);
}

void util_put_ulong(u32 val, u08 *buf)
{
  buf[0] = (u08)(val & 0xff);
  buf[1] = (u08)(val >> 8);
  buf[2] = (u08)(val >> 16);
  buf[3] = (u08)(val >> 24);
}

u32  util_get_ulong(u08 *buf)
{
  return buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}
