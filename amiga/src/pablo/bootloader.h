#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "parbox.h"

struct bootinfo {
  /* bootloader */
  ULONG   rom_size;
  UWORD   page_size;
  UWORD   bl_version;
  UWORD   bl_mach_tag;
  /* rom image */
  UWORD   rom_version;
  UWORD   rom_mach_tag;
  UWORD   rom_crc;
};
typedef struct bootinfo bootinfo_t;

/* ro registers defined in bootloader */
#define BOOTLOADER_RO_BL_VERSION        0

/* magic bit to detect bootloader in version info */
#define BOOTLOADER_VER_TAG              0x8000

#define BOOTLOADER_RET_MASK             0xf0
#define BOOTLOADER_RET_OK               0x00
#define BOOTLOADER_RET_NO_PING          0x10
#define BOOTLOADER_RET_NO_BOOTLOADER    0x20
#define BOOTLOADER_RET_REG_RO_ERROR     0x30
#define BBOTLOADER_RET_NO_BOOTLOADER    0x40


extern int bootloader_enter(parbox_handle_t *pb, bootinfo_t *bi);
extern int bootloader_leave(parbox_handle_t *pb, bootinfo_t *bi);

extern const char *bootloader_perror(int res);

#endif
