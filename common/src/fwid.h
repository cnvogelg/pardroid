#ifndef FWID_H
#define FWID_H

// firmware ids
#define FWID_TEST_PAMELA          0x9000
#define FWID_TEST_PALOMA          0x9001

extern void fwid_decode(u16 fw_id, rom_pchar *res);

#endif
