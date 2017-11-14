#include "autoconf.h"
#include "types.h"

#include "pario_pins.h"
#include "proto_low.h"

/* signal names:
   IN
      POUT -> clk
      SELECT -> cflg
      STROBE -> strobe
   OUT
      ACK -> ack
      BUSY -> rak
*/

#define ddr(x)    pario_data_ddr(x)
#define dout(x)   pario_set_data(x)
#define din()     pario_get_data()

#define clk()     pario_get_pout()
#define cflg()    pario_get_select()
#define strobe()  pario_get_strobe()

#define rak_lo()  pario_busy_lo()
#define rak_hi()  pario_busy_hi()
#define ack_lo()  pario_ack_lo()
#define ack_hi()  pario_ack_hi()


void proto_low_init(u08 status)
{
    pario_init();
}

u08 proto_low_get_cmd(void)
{
  // clock is low -> no command
  if(clk()) {
    return 0xff;
  }

  // read data (command nybble)
  u08 cmd = din() & 0x0f;

  // 5bit: cflag
  if(cflg()) {
    cmd |= 0x10;
  }

  return cmd;
}

void proto_low_action(void)
{
  rak_lo();
}

void proto_low_end(u08 status)
{
  u08 val = (status & 0xf0) | 0xf;
  dout(val);
  while(!clk()) {}
  rak_hi();
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
  ack_lo();
}

void proto_low_ack_hi(void)
{
  ack_hi();
}

u08  proto_low_set_status(u08 status)
{
  return 0;
}

void proto_low_wait_cflg_hi(void)
{
  while(!cflg()) {}
}
