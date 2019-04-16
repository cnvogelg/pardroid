#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "pamela.h"
#include "pblfile.h"

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

extern int bootloader_enter(pamela_handle_t *pb, bootinfo_t *bi);
extern int bootloader_leave(pamela_handle_t *pb);

extern int bootloader_update_fw_info(pamela_handle_t *pb, bootinfo_t *bi);
extern int bootloader_check_file(bootinfo_t *bi, pblfile_t *pf);

extern int bootloader_flash(pamela_handle_t *pb, bootinfo_t *bi,
                            bl_flash_cb_t pre_callback,
                            void *user_data);
extern int bootloader_read(pamela_handle_t *pb, bootinfo_t *bi,
                           bl_read_cb_t pre_callback,
                           bl_read_cb_t post_callback,
                           void *user_data);

extern const char *bootloader_perror(int res);

#endif
