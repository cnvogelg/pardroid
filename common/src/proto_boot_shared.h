/* bootloader commands */
#ifndef PROTO_BOOT_SHARED_H
#define PROTO_BOOT_SHARED_H

/* bootloader uses device mode and therefore its
   commands start at 0x20 */
#define PROTO_BOOT_CMD_RWORD_PAGE_SIZE            0x20
#define PROTO_BOOT_CMD_RLONG_ROM_SIZE             0x21
#define PROTO_BOOT_CMD_RWORD_ROM_CRC              0x22
#define PROTO_BOOT_CMD_RWORD_ROM_MACH_TAG         0x23
#define PROTO_BOOT_CMD_RWORD_ROM_FW_VERSION       0x24
#define PROTO_BOOT_CMD_RWORD_ROM_FW_ID            0x25
#define PROTO_BOOT_CMD_WLONG_PAGE_ADDR            0x26
#define PROTO_BOOT_CMD_WBLOCK_PAGE_WRITE          0x27
#define PROTO_BOOT_CMD_RBLOCK_PAGE_READ           0x28

#endif