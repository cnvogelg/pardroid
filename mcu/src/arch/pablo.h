#ifndef PABLO_H
#define PABLO_H

u16 pablo_get_mach_tag(void);
u16 pablo_get_rom_version(void);
u16 pablo_get_rom_fw_id(void);
u16 pablo_get_rom_crc(void);
u16 pablo_check_rom_crc(void);

#endif
