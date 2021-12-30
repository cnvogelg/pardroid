#ifndef PROTO_BOOT_H
#define PROTO_BOOT_H

#include "proto_dev.h"

/* init/exit of io mode */
extern proto_handle_t *proto_boot_init(proto_env_handle_t *penv);
extern void proto_boot_exit(proto_handle_t *ph);

/* rom info */
extern int proto_boot_get_page_size(proto_handle_t *ph, UWORD *page_size);
extern int proto_boot_get_rom_size(proto_handle_t *ph, ULONG *rom_size);

/* flashed rom info */
extern int proto_boot_get_rom_fw_id(proto_handle_t *ph, UWORD *result);
extern int proto_boot_get_rom_fw_version(proto_handle_t *ph, UWORD *result);
extern int proto_boot_get_rom_mach_tag(proto_handle_t *ph, UWORD *result);
extern int proto_boot_get_rom_crc(proto_handle_t *ph, UWORD *result);

/* flash/verify ops */
extern int proto_boot_set_page_addr(proto_handle_t *ph, ULONG addr);
extern int proto_boot_write_page(proto_handle_t *ph, BYTE *data, UWORD page_size);
extern int proto_boot_read_page(proto_handle_t *ph, BYTE *data, UWORD page_size);

#endif
