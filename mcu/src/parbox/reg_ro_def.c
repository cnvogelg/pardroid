#include "types.h"
#include "arch.h"
#include "machtag.h"
#include "reg_ro_def.h"

// proto values
const u16 ro_version ROM_ATTR = VERSION_TAG;
const u16 ro_machtag ROM_ATTR = MACHTAG;

u16 ro_num_regs(void) {
  return reg_ro_table_size << 8 | 2;
}
