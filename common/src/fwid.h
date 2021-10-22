#ifndef FWID_H
#define FWID_H

#define FWID_BOOTLOADER_PABLO     0x8000

// firmware ids
#define FWID_TEST_PAMELA          0x4000
#define FWID_TEST_PROTO           0x4001
#define FWID_TEST_PROTO_ATOM      0x4002

extern void fwid_decode(u16 fw_id, rom_pchar *res);

#endif
