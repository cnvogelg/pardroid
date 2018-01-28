#include "types.h"
#include "arch.h"
#include "fwid.h"

void fwid_decode(u16 fw_id, rom_pchar *res)
{
  *res = PSTR("?");
  switch(fw_id) {
    case FWID_TEST_PAMELA:
      *res = PSTR("test-pamela");
      break;
    case FWID_TEST_PALOMA:
      *res = PSTR("test-paloma");
      break;
  }
}
