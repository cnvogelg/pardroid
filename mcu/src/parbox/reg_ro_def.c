#include "types.h"
#include "arch.h"
#include "machtag.h"
#include "reg_ro_def.h"
#include "reg_rw_def.h"

// proto values
const u16 ro_fw_version ROM_ATTR = VERSION_TAG;
const u16 ro_fw_machtag ROM_ATTR = MACHTAG;

u16 ro_num_regs(void) {
  return reg_ro_table_size << 8 | reg_rw_table_size;
}
