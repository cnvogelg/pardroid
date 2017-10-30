#include "types.h"
#include "arch.h"
#include "machtag.h"
#include "reg_def.h"
#include "status.h"

// proto values
const u16 fw_version ROM_ATTR = VERSION_TAG;
const u16 fw_machtag ROM_ATTR = MACHTAG;

// must be define by application
extern const u16 fw_id ROM_ATTR;

REG_TABLE_BEGIN(base)
  REG_TABLE_RO_ROM_W(fw_version),
  REG_TABLE_RO_ROM_W(fw_machtag),
  REG_TABLE_RO_ROM_W(fw_id),
  REG_TABLE_RO_FUNC(reg_get_error)
REG_TABLE_END(base, 0, 0)

void reg_get_error(u16 *valp, u08 mode)
{
  u08 e = status_clear_error();
  *valp = e;
}
