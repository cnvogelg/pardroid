#include "autoconf.h"
#include "types.h"

#include "proto_low.h"

void proto_low_init(u08 status)
{
}

u08 proto_low_get_cmd(void)
{
  return 0;
}

void proto_low_action(void)
{
}

void proto_low_end(u08 status)
{
}

void proto_low_read_word(u16 v)
{
}

u16  proto_low_write_word(void)
{
  return 0;
}

void proto_low_read_long(u32 v)
{
}

u32  proto_low_write_long(void)
{
  return 0;
}

u16  proto_low_write_block(u16 max_words, u08 *buffer, u16 *chn_ext)
{
  return 0;
}

u08  proto_low_read_block(u16 num_words, u08 *buffer, u16 chn_ext)
{
  return 0;
}

void proto_low_ack_lo(void)
{
}

void proto_low_ack_hi(void)
{
}

u08  proto_low_set_status(u08 status)
{
  return 0;
}

void proto_low_wait_cflg_hi(void)
{
}
