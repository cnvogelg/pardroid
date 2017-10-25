#include "types.h"
#include "arch.h"
#include "machtag.h"
#include "reg_def.h"
#include "status.h"

// proto values
const u16 fw_version ROM_ATTR = VERSION_TAG;
const u16 fw_machtag ROM_ATTR = MACHTAG;

void reg_get_error(u16 *valp, u08 mode)
{
  u08 e = status_clear_error();
  *valp = e;
}

