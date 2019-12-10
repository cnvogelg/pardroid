#ifndef FW_H
#define FW_H

#include "machtag.h"
#include "arch.h"

#define FW_INFO(id, version) \
  const u16 fw_id = id; \
  const u16 fw_version = version; \
  const u16 fw_machtag = MACHTAG;

#define FW_GET_ID()       read_rom_word(&fw_id)
#define FW_GET_VERSION()  read_rom_word(&fw_version)
#define FW_GET_MACHTAG()  read_rom_word(&fw_machtag)

extern const u16 fw_id;
extern const u16 fw_version;
extern const u16 fw_machtag;

#endif