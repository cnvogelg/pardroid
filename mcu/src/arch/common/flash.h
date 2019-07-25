#ifndef FLASH_H
#define FLASH_H

void flash_program_page(flash_size_t address, u08 *data);
void flash_read_page(flash_size_t address, u08 *data);

#endif
