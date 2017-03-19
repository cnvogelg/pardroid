#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "parbox.h"
#include "pblfile.h"

struct bootinfo {
  /* bootloader */
  ULONG   rom_size;
  UWORD   page_size;
  UWORD   bl_version;
  UWORD   bl_mach_tag;
  /* rom image */
  UWORD   fw_version;
  UWORD   fw_mach_tag;
  UWORD   fw_crc;
};
typedef struct bootinfo bootinfo_t;

/* ro registers defined in bootloader */
#define BOOTLOADER_RO_BL_VERSION        0
#define BOOTLOADER_RO_BL_MACHTAG        1
#define BOOTLOADER_RO_PAGE_SIZE         2
#define BOOTLOADER_RO_ROM_SIZE          3
#define BOOTLOADER_RO_FW_CRC            4
#define BOOTLOADER_RO_FW_MACHTAG        5
#define BOOTLOADER_RO_FW_VERSION        6

/* rw registers */
#define BOOTLOADER_RW_PAGE_ADDR         0

/* channels */
#define BOOTLOADER_CHN_PAGES            0

/* magic bit to detect bootloader in version info */
#define BOOTLOADER_VER_TAG              0x8000

#define BOOTLOADER_RET_MASK                 0xf0
#define BOOTLOADER_RET_OK                   0x00
#define BOOTLOADER_RET_NO_PING              0x10
#define BOOTLOADER_RET_NO_BOOTLOADER        0x20
#define BOOTLOADER_RET_REG_RO_ERROR         0x30
#define BOOTLOADER_RET_NO_FIRMWARE          0x40
#define BOOTLOADER_RET_INVALID_FILE         0x50
#define BOOTLOADER_RET_WRONG_FILE_SIZE      0x60
#define BOOTLOADER_RET_WRONG_FILE_MACHTAG   0x70
#define BOOTLOADER_RET_FAILED_SET_ADDR      0x80
#define BOOTLOADER_RET_WRITE_PAGE_ERROR     0x90
#define BOOTLOADER_RET_READ_PAGE_ERROR      0xa0
#define BOOTLOADER_RET_NO_PAGE_DATA         0xb0
#define BOOTLOADER_RET_DATA_MISMATCH        0xc0

/* update callback for flash/verify ops */
struct bl_flash_data {
  ULONG   addr;
  ULONG   max_addr;
  UBYTE   *buffer;
  UWORD   buffer_size;
};
typedef struct bl_flash_data bl_flash_data_t;
typedef struct bl_flash_data bl_read_data_t;

typedef int (*bl_flash_cb_t)(bl_flash_data_t *data, void *user_data);
typedef int (*bl_read_cb_t)(bl_read_data_t *data, void *user_data);

extern int bootloader_enter(parbox_handle_t *pb, bootinfo_t *bi);
extern int bootloader_leave(parbox_handle_t *pb);

extern int bootloader_update_fw_info(parbox_handle_t *pb, bootinfo_t *bi);
extern int bootloader_check_file(bootinfo_t *bi, pblfile_t *pf);

extern int bootloader_flash(parbox_handle_t *pb, bootinfo_t *bi,
                            bl_flash_cb_t pre_callback,
                            void *user_data);
extern int bootloader_read(parbox_handle_t *pb, bootinfo_t *bi,
                           bl_read_cb_t pre_callback,
                           bl_read_cb_t post_callback,
                           void *user_data);

extern const char *bootloader_perror(int res);

#endif
