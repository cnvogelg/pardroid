#ifndef FLASH_H
#define FLASH_H

void flash_program_page(uint16_t address, const uint8_t *data);
void flash_read_page(uint16_t address, uint8_t *data);
uint16_t flash_check_crc(void);

#endif
