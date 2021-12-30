#ifndef PABLO_H
#define PABLO_H

extern u16 pablo_get_mach_tag(void);
extern u16 pablo_get_rom_version(void);
extern u16 pablo_get_rom_fw_id(void);
extern u16 pablo_get_rom_crc(void);
extern u16 pablo_check_rom_crc(void);

#endif
