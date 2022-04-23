#ifndef FWID_H
#define FWID_H

#define FWID_BOOTLOADER_PABLO     0x8000

// firmware ids
#define FWID_TEST_BASE            0x4000
#define FWID_TEST_PROTO           0x4001
#define FWID_TEST_PROTO_ATOM      0x4002
#define FWID_TEST_PROTO_DEV       0x4003
#define FWID_TEST_PROTO_IO        0x4004
#define FWID_TEST_PROTO_BOOT      0x4005
#define FWID_TEST_PAMELA          0x4006
#define FWID_TEST_SDCARD          0x4007
#define FWID_TEST_ENC28J60        0x4008
#define FWID_TEST_WIZNET          0x4009

extern void fwid_decode(u16 fw_id, rom_pchar *res);

#endif
