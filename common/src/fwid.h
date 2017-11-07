#ifndef FWID_H
#define FWID_H

// firmware ids
#define FWID_TEST_BASE            0x8001
#define FWID_TEST_PROTO           0x8002
#define FWID_TEST_HANDLER         0x8003

extern void fwid_decode(u16 fw_id, rom_pchar *res);

#endif
