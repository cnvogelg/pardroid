#ifndef FW_H
#define FW_H

#include "machtag.h"
#include "arch.h"

struct fw_info_s {
  u32 magic;
  u16 id;
  u16 version;
  u16 machtag;
};

#define FW_MAGIC 0x46574944 // 'FWID'

#define FW_INFO(id, version) \
  const struct fw_info_s fw_info ROM_ATTR = { \
    FW_MAGIC, id, version, MACHTAG \
  };

#define FW_GET_ID()       read_rom_word(&fw_info.id)
#define FW_GET_VERSION()  read_rom_word(&fw_info.version)
#define FW_GET_MACHTAG()  read_rom_word(&fw_info.machtag)

extern const struct fw_info_s fw_info ROM_ATTR;

#endif