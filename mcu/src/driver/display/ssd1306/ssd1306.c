#include "autoconf.h"
#include "arch.h"
#include "types.h"
#include "i2c.h"
#include "ssd1306.h"
#include "font6x8.h"

#define WIDTH               128
#define HEIGHT              32
#define PAGES               (HEIGHT / 8)
#define FRAME_SIZE          (WIDTH * PAGES)

#define SET_CONTRAST        0x81
#define SET_ENTIRE_ON       0xa4
#define SET_NORM_INV        0xa6
#define SET_DISP            0xae
#define SET_MEM_ADDR        0x20
#define SET_COL_ADDR        0x21
#define SET_PAGE_ADDR       0x22
#define SET_DISP_START_LINE 0x40
#define SET_SEG_REMAP       0xa0
#define SET_MUX_RATIO       0xa8
#define SET_COM_OUT_DIR     0xc0
#define SET_DISP_OFFSET     0xd3
#define SET_COM_PIN_CFG     0xda
#define SET_DISP_CLK_DIV    0xd5
#define SET_PRECHARGE       0xd9
#define SET_VCOM_DESEL      0xdb
#define SET_CHARGE_PUMP     0x8d

u08 ssd1306_scan(void)
{
  // try 0x3c or 0x3d address
  u08 test = 0;
  if(i2c_write(0x3c, &test, 1) == 0) {
    return 0x3c;
  }
  if(i2c_write(0x3d, &test, 1) == 0) {
    return 0x3d;
  }
  return 0;
}

static u08 write_cmd(u08 addr, u08 cmd)
{
  // 0x80 = Co=1, D/C//=0
  u08 buf[2] = { 0x80, cmd };
  return i2c_write(addr, buf, 2);
}

static u08 write_cmd3(u08 addr, u08 cmd, u08 a, u08 b)
{
  u08 res = write_cmd(addr, cmd);
  if(res) return res;
  res = write_cmd(addr, a);
  if(res) return res;
  res = write_cmd(addr, b);
  return res;
}

static u08 write_data_begin(u08 addr)
{
  u08 res;

  res = i2c_start(addr, 1);
  if(res)
    return res;

  // 0x40 = Co=0, D/C=1
  res = i2c_write_byte(0x40);
  if(res)
    return res;

  return 0;
}

static void write_data_end(void)
{
  i2c_stop();
}

#if 0
static u08 write_data_bytes(const u08 *data, u16 len)
{
  for(u16 i=0;i<16;i++) {
    u08 res = i2c_write_byte(data[i]);
    if(res)
      return res;
  }
  return 0;
}

static u08 write_data(u08 addr, const u08 *data, u16 len)
{
  u08 res = write_data_begin(addr);
  if(res)
    return res;
  res = write_data_bytes(data, len);
  if(res)
    return res;
  write_data_end();
  return 0;
}
#endif

const u08 init_cmds[] ROM_ATTR = {
  SET_DISP | 0x00, // off
  // address setting
  SET_MEM_ADDR, 0x00, // horizontal
  // resolution and layout
  SET_DISP_START_LINE | 0x00,
  SET_SEG_REMAP | 0x01, // column addr 127 mapped to SEG0
  SET_MUX_RATIO, HEIGHT - 1,
  SET_COM_OUT_DIR | 0x08, // scan from COM[N] to COM0
  SET_DISP_OFFSET, 0x00,
  SET_COM_PIN_CFG, 0x02, // 0x02 if HEIGHT == 32 or HEIGHT == 16 else 0x12,
  // timing and driving scheme
  SET_DISP_CLK_DIV, 0x80,
  SET_PRECHARGE, 0xf1, // 0x22 if self.external_vcc else 0xf1,
  SET_VCOM_DESEL, 0x30, // 0.83*Vcc
  // display
  SET_CONTRAST, 0xff, // maximum
  SET_ENTIRE_ON, // output follows RAM contents
  SET_NORM_INV, // not inverted
  // charge pump
  SET_CHARGE_PUMP, 0x14, // 0x10 if self.external_vcc else 0x14,
  SET_DISP | 0x01 // on
};

u08 ssd1306_init(u08 addr)
{
  u08 res;

  for(u08 i=0;i<sizeof(init_cmds);i++) {
    u08 cmd = read_rom_char(init_cmds + i);
    res = write_cmd(addr, cmd);
    if(res)
      return res;
  }

  return 0;
}

static u08 set_draw_range(u08 addr, u08 x_min, u08 x_max, u08 p_min, u08 p_max)
{
  u08 res;

  res = write_cmd3(addr, SET_COL_ADDR, x_min, x_max);
  if(res) return res;

  // pages = height / 8
  res = write_cmd3(addr, SET_PAGE_ADDR, p_min, p_max);
  return res;
}

u08 ssd1306_clear(u08 addr)
{
  u08 res = set_draw_range(addr, 0, WIDTH-1, 0, PAGES-1);
  if(res) return res;

  res = write_data_begin(addr);
  if(res) return res;

  for(u16 i=0;i<(WIDTH * PAGES); i++) {
    i2c_write_byte(0);
  }

  write_data_end();
  return 0;
}

#define FONT_WIDTH 6

u08 ssd1306_begin_txt(u08 addr, u08 x_pos, u08 y_pos, u08 len)
{
  u08 res;

  x_pos *= FONT_WIDTH;
  len *= FONT_WIDTH;

  u08 x_max = x_pos + len - 1;

  res = set_draw_range(addr, x_pos, x_max, y_pos, y_pos);
  if(res) return res;
  return write_data_begin(addr);
}

u08 ssd1306_write_char(u08 chr)
{
  if((chr < 32) || (chr > 127)) {
    chr = '?';
  }
  chr -= 32;

  rom_pchar char_data = (rom_pchar)font6x8 + chr * FONT_WIDTH;
  for(u08 i=0;i<FONT_WIDTH;i++) {
    u08 data = read_rom_char(char_data);
    char_data++;
    i2c_write_byte(data);
  }
  return 0;
}

void ssd1306_end_txt(void)
{
  write_data_end();
}
