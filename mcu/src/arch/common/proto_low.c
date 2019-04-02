#include "autoconf.h"
#include "types.h"

#include "pario_pins.h"
#include "proto_low.h"

/* signal names:
   IN
      SELECT -> clk
      STROBE -> strobe
   OUT
      ACK -> signal
      POUT -> rak
*/

#define ddr(x)    pario_data_ddr(x)
#define dout(x)   pario_set_data(x)
#define din()     pario_get_data()

#define clk()     pario_get_select()
#define strobe()  pario_get_strobe()

#define rak_lo()  pario_pout_lo()
#define rak_hi()  pario_pout_hi()
#define ack_lo()  pario_ack_lo()
#define ack_hi()  pario_ack_hi()

#define wait_clk_hi()  while(!clk()) {}
#define wait_clk_lo()  while(clk()) {}
#define ddr_in()       ddr(0)
#define ddr_out()      ddr(0xff)

void proto_low_init(void)
{
    pario_init();

    dout(0xff);
    ddr_in();
}

u08 proto_low_get_cmd(void)
{
  // clock is low -> no command
  if(clk()) {
    return 0xff;
  }

  // read data (command nybble)
  return din();
}

void proto_low_action(void)
{
  rak_lo();
}

void proto_low_end(void)
{
  wait_clk_hi();
  rak_hi();
}

void proto_low_read_word(u16 v)
{
  u08 a = (u08)(v >> 8);
  u08 b = (u08)(v & 0xff);

  irq_off();

  rak_lo();
  wait_clk_hi();
  ddr_out();

  wait_clk_lo();
  dout(a);
  wait_clk_hi();
  dout(b);

  wait_clk_lo();
  ddr_in();

  irq_on();
}

u16  proto_low_write_word(void)
{
  irq_off();

  rak_lo();

  wait_clk_hi();
  u08 a = din();
  wait_clk_lo();
  u08 b = din();

  irq_on();
  return (a << 8) | b;
}

void proto_low_read_long(u32 v)
{
  u08 a = (u08)(v >> 24);
  u08 b = (u08)(v >> 16);
  u08 c = (u08)(v >> 8);
  u08 d = (u08)(v & 0xff);

  irq_off();

  rak_lo();
  wait_clk_hi();
  ddr_out();

  wait_clk_lo();
  dout(a);
  wait_clk_hi();
  dout(b);
  wait_clk_lo();
  dout(c);
  wait_clk_hi();
  dout(d);

  wait_clk_lo();
  ddr_in();

  irq_on();
}

u32  proto_low_write_long(void)
{
  irq_off();

  rak_lo();

  wait_clk_hi();
  u08 a = din();
  wait_clk_lo();
  u08 b = din();
  wait_clk_hi();
  u08 c = din();
  wait_clk_lo();
  u08 d = din();

  irq_on();
  return (a << 24) | (b << 16) | (c << 8) | d;
}

u16  proto_low_write_block(u16 max_words, u08 *buffer, u16 *chn_ext)
{
  irq_off();

  rak_lo();

  wait_clk_hi();
  u08 eh = din();
  wait_clk_lo();
  u08 el = din();

  wait_clk_hi();
  u08 sh = din();
  wait_clk_lo();
  u08 sl = din();

  u16 size = (sh << 8) | sl;
  if(size > max_words) {
    rak_hi();
    goto write_end;
  }

  for(u16 i=0;i<size;i++) {
    wait_clk_hi();
    *buffer++ = din();
    wait_clk_lo();
    *buffer++ = din();
  }

write_end:

  irq_on();

  *chn_ext = (eh << 8) | el;
  return size;
}

void proto_low_read_block(u16 num_words, u08 *buffer, u16 chn_ext)
{
  u08 eh = (u08)(chn_ext >> 8);
  u08 el = (u08)(chn_ext & 0xff);
  u08 sh = (u08)(num_words >> 8);
  u08 sl = (u08)(num_words & 0xff);

  irq_off();

  rak_lo();
  wait_clk_hi();
  ddr_out();

  wait_clk_lo();
  dout(eh);
  wait_clk_hi();
  dout(el);

  wait_clk_lo();
  dout(sh);
  wait_clk_hi();
  dout(sl);

  u16 i=0;
  for(i=0;i<num_words;i++) {
    wait_clk_lo();
    dout(*buffer++);
    wait_clk_hi();
    dout(*buffer++);
  }

  wait_clk_lo();
  ddr_in();

  irq_on();
}

void proto_low_ack_lo(void)
{
  ack_lo();
}

void proto_low_ack_hi(void)
{
  ack_hi();
}
