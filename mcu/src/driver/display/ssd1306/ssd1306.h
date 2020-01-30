/*
 * SSD1306 display driver via i2c
 */

#ifndef SSD1306_H
#define SSD1306_H

extern u08 ssd1306_scan(void);
extern u08 ssd1306_init(u08 addr);
extern u08 ssd1306_clear(u08 addr);

extern u08 ssd1306_begin_txt(u08 addr, u08 x_pos, u08 y_pos, u08 len);
extern u08 ssd1306_write_char(u08 chr);
extern void ssd1306_end_txt(void);

#endif