#ifndef PROTO_BOOT_H
#define PROTO_BOOT_H

#define PROTO_BOOT_INIT_PROTO   0
#define PROTO_BOOT_INIT_APP     1

extern int  proto_boot_init(void);
extern void proto_boot_handle_cmd(void);

// ----- API -----
extern u16 proto_boot_api_get_page_size(void);
extern u32 proto_boot_api_get_rom_size(void);

extern u16 proto_boot_api_get_rom_crc(void);
extern u16 proto_boot_api_get_rom_mach_tag(void);
extern u16 proto_boot_api_get_rom_fw_version(void);
extern u16 proto_boot_api_get_rom_fw_id(void);

extern void proto_boot_api_set_page_addr(u32 addr);
extern void proto_boot_api_get_page_read_buf(u08 **buf, u16 *size);
extern void proto_boot_api_get_page_write_buf(u08 **buf, u16 *size);

extern void proto_boot_api_flash_page(void);

#endif