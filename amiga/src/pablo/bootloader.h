#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "proto_env.h"
#include "pblfile.h"
#include "proto_atom.h"

struct bootinfo {
  /* bootloader */
  ULONG   rom_size;
  UWORD   page_size;
  UWORD   bl_version;
  UWORD   bl_mach_tag;
  /* rom image */
  UWORD   fw_id;
  UWORD   fw_version;
  UWORD   fw_mach_tag;
  UWORD   fw_crc;
};
typedef struct bootinfo bootinfo_t;

struct boot_handle {
    bootinfo_t  info;
    proto_handle_t *proto;
};
typedef struct boot_handle boot_handle_t;

/* channels */
#define BOOTLOADER_CHN_PAGES            0

/* magic bit to detect bootloader in version info */
#define BOOTLOADER_VER_TAG              0x8000

#define BOOTLOADER_RET_MASK                 0xf0
#define BOOTLOADER_RET_PROTO_MASK           0x0f

#define BOOTLOADER_RET_OK                   0x00
#define BOOTLOADER_RET_NO_RESET             0x10
#define BOOTLOADER_RET_NO_BOOTLOADER        0x20
#define BOOTLOADER_RET_NO_BOOTLOADER_MAGIC  0x30
#define BOOTLOADER_RET_READ_ERROR           0x40
#define BOOTLOADER_RET_NO_FIRMWARE          0x50
#define BOOTLOADER_RET_INVALID_FILE         0x60
#define BOOTLOADER_RET_WRONG_FILE_SIZE      0x70
#define BOOTLOADER_RET_WRONG_FILE_MACHTAG   0x80
#define BOOTLOADER_RET_FAILED_SET_ADDR      0x90
#define BOOTLOADER_RET_WRITE_PAGE_ERROR     0xa0
#define BOOTLOADER_RET_READ_PAGE_ERROR      0xb0
#define BOOTLOADER_RET_NO_PAGE_DATA         0xc0
#define BOOTLOADER_RET_DATA_MISMATCH        0xd0

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

extern int bootloader_init(proto_env_handle_t *pb, boot_handle_t *bh);
extern int bootloader_leave(boot_handle_t *bh);
extern void bootloader_exit(boot_handle_t *bh);

extern int bootloader_update_fw_info(boot_handle_t *bh);
extern int bootloader_check_file(boot_handle_t *bh, pblfile_t *pf);

extern int bootloader_flash(boot_handle_t *bh,
                            bl_flash_cb_t pre_callback,
                            void *user_data);
extern int bootloader_read(boot_handle_t *bh,
                           bl_read_cb_t pre_callback,
                           bl_read_cb_t post_callback,
                           void *user_data);

extern const char *bootloader_perror(int res);

#endif
