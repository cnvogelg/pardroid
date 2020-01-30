#ifndef DISPLAY_H
#define DISPLAY_H

#define DISPLAY_OK    0
#define DISPLAY_NONE  99

extern u08 display_init(void);
extern u08 display_clear(void);
extern void display_setpos(u08 x, u08 y);
extern u08 display_printp(rom_pchar str);
extern u08 display_print(const char *str);

#endif
