#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pario.h"
#include "timer.h"
#include "proto_atom.h"
#include "proto_dev.h"
#include "proto_boot.h"
#include "proto_boot_shared.h"

proto_handle_t *proto_boot_init(proto_env_handle_t *penv)
{
  proto_handle_t *ph = proto_atom_init(penv);
  if(ph == NULL) {
    return NULL;
  }

  // trigger reset to enter bootloader
  int res = proto_dev_action_bootloader(ph);
  if(res != PROTO_RET_OK) {
    proto_atom_exit(ph);
    return NULL;
  }

  return ph;
}

int proto_boot_leave(proto_handle_t *ph)
{
  // trigger knok mode to return to save state of device
  return proto_dev_action_knok(ph);
}

void proto_boot_exit(proto_handle_t *ph)
{
  proto_atom_exit(ph);
}

/* rom info */
int proto_boot_get_page_size(proto_handle_t *ph, UWORD *page_size)
{
  return proto_atom_read_word(ph, PROTO_BOOT_CMD_RWORD_PAGE_SIZE, page_size);
}

int proto_boot_get_rom_size(proto_handle_t *ph, ULONG *rom_size)
{
  return proto_atom_read_long(ph, PROTO_BOOT_CMD_RLONG_ROM_SIZE, rom_size);
}

/* flashed rom info */
int proto_boot_get_rom_fw_id(proto_handle_t *ph, UWORD *result)
{
  return proto_atom_read_word(ph, PROTO_BOOT_CMD_RWORD_ROM_FW_ID, result);
}

int proto_boot_get_rom_fw_version(proto_handle_t *ph, UWORD *result)
{
  return proto_atom_read_word(ph, PROTO_BOOT_CMD_RWORD_ROM_FW_VERSION, result);
}

int proto_boot_get_rom_mach_tag(proto_handle_t *ph, UWORD *result)
{
  return proto_atom_read_word(ph, PROTO_BOOT_CMD_RWORD_ROM_MACH_TAG, result);
}

int proto_boot_get_rom_crc(proto_handle_t *ph, UWORD *result)
{
  return proto_atom_read_word(ph, PROTO_BOOT_CMD_RWORD_ROM_CRC, result); 
}

/* flash/verify ops */
int proto_boot_set_page_addr(proto_handle_t *ph, ULONG addr)
{
  return proto_atom_write_long(ph, PROTO_BOOT_CMD_WLONG_PAGE_ADDR, addr);
}

int proto_boot_write_page(proto_handle_t *ph, BYTE *data, UWORD page_size)
{
  return proto_atom_write_block(ph, PROTO_BOOT_CMD_WBLOCK_PAGE_WRITE, data, page_size);
}

int proto_boot_read_page(proto_handle_t *ph, BYTE *data, UWORD page_size)
{
  return proto_atom_read_block(ph, PROTO_BOOT_CMD_RBLOCK_PAGE_READ, data, page_size);
}
