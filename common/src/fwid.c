#include "types.h"
#include "arch.h"
#include "fwid.h"

void fwid_decode(u16 fw_id, rom_pchar *res)
{
  *res = PSTR("?");
  switch(fw_id) {
    case FWID_TEST_BASE:
      *res = PSTR("test-base");
      break;
    case FWID_TEST_PROTO:
      *res = PSTR("test-proto");
      break;
    case FWID_TEST_HANDLER:
      *res = PSTR("test-handler");
      break;
  }
}
