#include "autoconf.h"
#include "arch.h"
#include "types.h"

#include "hw_i2c.h"

#include "ssd1306.h"
#include "display.h"

static u08 ssd_id;
static u08 xpos, ypos;

u08 display_init(void)
{
  u08 res = DISPLAY_NONE;
  ssd_id = ssd1306_scan();
  if(ssd_id) {
    res = ssd1306_init(ssd_id);
    if(res != 0) {
      ssd_id = 0;
    }
  }
  xpos = 0;
  ypos = 0;
  return res;
}

u08 display_clear(void)
{
  if(ssd_id) {
    return ssd1306_clear(ssd_id);
  } else {
    return DISPLAY_NONE;
  }
}

void display_setpos(u08 x, u08 y)
{
  xpos = x;
  ypos = y;
}

u08 display_printp(rom_pchar str)
{
  if(!ssd_id) {
    return DISPLAY_NONE;
  }

  // calc len
  u08 len = 0;
  rom_pchar ptr = str;
  while(read_rom_char(ptr)!=0) {
    len++;
    ptr++;
  }

  u08 res = ssd1306_begin_txt(ssd_id, xpos, ypos, len);
  if(res != HW_I2C_OK) return DISPLAY_ERROR;
  ptr = str;
  u08 data;
  while((data=read_rom_char(ptr))!=0) {
    res = ssd1306_write_char(ssd_id, data);
    if(res) return res;
    ptr++;
  }

  xpos += len;
  return DISPLAY_OK;
}

u08 display_print(const char *str)
{
  if(!ssd_id) {
    return DISPLAY_NONE;
  }

  // calc len
  u08 len = 0;
  const char *ptr = str;
  while(*ptr!=0) {
    len++;
    ptr++;
  }

  u08 res = ssd1306_begin_txt(ssd_id, xpos, ypos, len);
  if(res) return res;
  ptr = str;
  u08 data;
  while((data=*ptr)!=0) {
    res = ssd1306_write_char(ssd_id, data);
    if(res) return res;
    ptr++;
  }

  xpos += len;
  return DISPLAY_OK;
}
