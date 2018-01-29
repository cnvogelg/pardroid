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

#define wait_clk_hi()  while(!clk()) {}
#define wait_clk_lo()  while(clk()) {}
#define ddr_in()       ddr(0)
#define ddr_out()      ddr(0xff)
#define ddr_idle()     ddr(0xf0)

void proto_low_init(u08 status)
{
    pario_init();

    u08 val = (status & 0xf0) | 0xf;
    dout(val);
    ddr_idle();
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
  if(!cflg()) {
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
  ddr_idle();

  irq_on();
}

u16  proto_low_write_word(void)
{
  irq_off();

  rak_lo();
  wait_clk_hi();
  ddr_in();

  wait_clk_lo();
  u08 a = din();
  wait_clk_hi();
  u08 b = din();

  wait_clk_lo();
  ddr_idle();

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
  ddr_idle();

  irq_on();
}

u32  proto_low_write_long(void)
{
  irq_off();

  rak_lo();
  wait_clk_hi();
  ddr_in();

  wait_clk_lo();
  u08 a = din();
  wait_clk_hi();
  u08 b = din();
  wait_clk_lo();
  u08 c = din();
  wait_clk_hi();
  u08 d = din();

  wait_clk_lo();
  ddr_idle();

  irq_on();
  return (a << 24) | (b << 16) | (c << 8) | d;
}

u16  proto_low_write_block(u16 max_words, u08 *buffer, u16 *chn_ext)
{
  irq_off();

  rak_lo();
  wait_clk_hi();
  ddr_in();

  wait_clk_lo();
  u08 eh = din();
  wait_clk_hi();
  u08 el = din();

  wait_clk_lo();
  u08 sh = din();
  wait_clk_hi();
  u08 sl = din();

  u16 size = (sh << 8) | sl;
  if(size > max_words) {
    rak_hi();
    goto write_end;
  }

  for(u16 i=0;i<size;i++) {
    wait_clk_lo();
    *buffer++ = din();
    wait_clk_hi();
    *buffer++ = din();
  }

write_end:
  wait_clk_lo();
  ddr_idle();

  irq_on();

  *chn_ext = (eh << 8) | el;
  return size;
}

u08  proto_low_read_block(u16 num_words, u08 *buffer, u16 chn_ext)
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

  // wait_clk_lo
  while(clk()) {
    // did master abort?
    if(!cflg()) {
      goto read_end;
    }
  }

  for(i=0;i<num_words;i++) {
    wait_clk_lo();
    dout(*buffer++);
    wait_clk_hi();
    dout(*buffer++);
  }
read_end:

  wait_clk_lo();
  ddr_idle();

  irq_on();

  return (i != num_words);
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
  // cflg has to be high
  // (otherwise host currently reads status)
  if(!cflg()) {
    return 0;
  }

  // set status
  u08 val = (status & 0xf0) | 0xf;
  dout(val);
  return 1;
}

void proto_low_wait_cflg_hi(void)
{
  while(!cflg()) {}
}
