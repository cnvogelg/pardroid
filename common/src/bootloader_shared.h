#ifndef BOOTLOADER_SHARED_H
#define BOOTLOADER_SHARED_H

// bootloader: read word
#define PROTO_WFUNC_READ_BOOT_FW_ID             0x00
#define PROTO_WFUNC_READ_BOOT_FW_VERSION        0x01
#define PROTO_WFUNC_READ_BOOT_MACHTAG           0x02
#define PROTO_WFUNC_READ_BOOT_ROM_FW_ID         0x03
#define PROTO_WFUNC_READ_BOOT_ROM_FW_VERSION    0x04
#define PROTO_WFUNC_READ_BOOT_ROM_MACHTAG       0x05
#define PROTO_WFUNC_READ_BOOT_ROM_CRC           0x06
#define PROTO_WFUNC_READ_BOOT_PAGE_WORDS        0x07

// bootloader: read long
#define PROTO_LFUNC_READ_BOOT_ROM_SIZE          0x00
// bootloader: write long
#define PROTO_LFUNC_WRITE_BOOT_PAGE_ADDR        0x00

#endif
