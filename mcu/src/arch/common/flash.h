#ifndef FLASH_H
#define FLASH_H

void flash_program_page(u32 address, u08 *data);
void flash_read_page(u32 address, u08 *data);

#endif
