#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "bootloader.h"
#include "bootloader_shared.h"
#include "proto.h"
#include "types.h"
#include "arch.h"
#include "fwid.h"

int bootloader_enter(proto_env_handle_t *pb, bootinfo_t *bi)
{
  int res;
  proto_handle_t *ph = proto_env_get_proto(pb);

  /* try to enter bootloader (ignored if already running */
  res = proto_bootloader(ph);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_NO_BOOTLOADER | res;
  }

  /* check bootloader magic */
  UWORD fw_id;
  res = proto_wfunc_read(ph, PROTO_WFUNC_READ_BOOT_FW_ID, &fw_id);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }
  if(fw_id != FWID_BOOTLOADER_PABLO) {
    return BOOTLOADER_RET_NO_BOOTLOADER_MAGIC;
  }

  /* read version tag */
  UWORD bl_version;
  res = proto_wfunc_read(ph, PROTO_WFUNC_READ_BOOT_FW_VERSION, &bl_version);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  bi->bl_version = bl_version;

  /* bootloader mach tag */
  res = proto_wfunc_read(ph, PROTO_WFUNC_READ_BOOT_MACHTAG, &bi->bl_mach_tag);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  /* page size */
  res = proto_wfunc_read(ph, PROTO_WFUNC_READ_BOOT_PAGE_WORDS, &bi->page_size);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }
  bi->page_size *= 2;

  /* rom size */
  res = proto_lfunc_read(ph, PROTO_LFUNC_READ_BOOT_ROM_SIZE, &bi->rom_size);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  return bootloader_update_fw_info(pb, bi);
}

int bootloader_update_fw_info(proto_env_handle_t *pb, bootinfo_t *bi)
{
  int res;
  proto_handle_t *ph = proto_env_get_proto(pb);

  /* firmware crc */
  res = proto_wfunc_read(ph, PROTO_WFUNC_READ_BOOT_ROM_CRC, &bi->fw_crc);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  /* firmware mach tag */
  res = proto_wfunc_read(ph, PROTO_WFUNC_READ_BOOT_ROM_MACHTAG, &bi->fw_mach_tag);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  /* firmware version */
  res = proto_wfunc_read(ph, PROTO_WFUNC_READ_BOOT_ROM_FW_VERSION, &bi->fw_version);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  /* firmware id */
  res = proto_wfunc_read(ph, PROTO_WFUNC_READ_BOOT_ROM_FW_ID, &bi->fw_id);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  return BOOTLOADER_RET_OK;
}

int bootloader_check_file(bootinfo_t *bi, pblfile_t *pf)
{
  if(pf == NULL) {
    return BOOTLOADER_RET_INVALID_FILE;
  }

  if(pf->mach_tag != bi->bl_mach_tag) {
    return BOOTLOADER_RET_WRONG_FILE_MACHTAG;
  }

  if(pf->rom_size != bi->rom_size) {
    return BOOTLOADER_RET_WRONG_FILE_SIZE;
  }

  return BOOTLOADER_RET_OK;
}

int bootloader_flash(proto_env_handle_t *pb, bootinfo_t *bi,
                     bl_flash_cb_t pre_flash_func,
                     void *user_data)
{
  int res;
  proto_handle_t *ph = proto_env_get_proto(pb);

  UWORD page_size = bi->page_size;
  UWORD page_words = page_size >> 1;
  ULONG rom_size = bi->rom_size;

  bl_flash_data_t bu;
  bu.addr = 0;
  bu.max_addr = rom_size;
  bu.buffer_size = page_size;
  bu.buffer = 0;

  /* flash loop */
  while(bu.addr < rom_size) {
    /* setup read buffer in func */
    res = pre_flash_func(&bu, user_data);
    if(res != BOOTLOADER_RET_OK) {
      return res;
    }

    UBYTE *data = bu.buffer;
    if(data == 0) {
      return BOOTLOADER_RET_NO_PAGE_DATA;
    }

    /* set addr in bootloader */
    res = proto_lfunc_write(ph, PROTO_LFUNC_WRITE_BOOT_PAGE_ADDR, bu.addr);
    if(res != PROTO_RET_OK) {
      return BOOTLOADER_RET_FAILED_SET_ADDR | res;
    }

    /* send flash page (and do flash) */
    res = proto_chn_msg_write(ph, BOOTLOADER_CHN_PAGES, data, page_words);
    if(res != PROTO_RET_OK) {
      return BOOTLOADER_RET_WRITE_PAGE_ERROR | res;
    }

    bu.addr += page_size;
    data += page_size;
  }

  return BOOTLOADER_RET_OK;
}

int bootloader_read(proto_env_handle_t *pb, bootinfo_t *bi,
                    bl_read_cb_t pre_read_func,
                    bl_read_cb_t post_read_func,
                    void *user_data)
{
  int res;
  proto_handle_t *ph = proto_env_get_proto(pb);

  UWORD page_size = bi->page_size;
  UWORD page_words = page_size >> 1;
  ULONG rom_size = bi->rom_size;

  bl_flash_data_t bu;
  bu.addr = 0;
  bu.max_addr = rom_size;
  bu.buffer_size = page_size;
  bu.buffer = 0;

  /* read loop */
  while(bu.addr < rom_size) {
    /* pre func sets up read buffer */
    res = pre_read_func(&bu, user_data);
    if(res != BOOTLOADER_RET_OK) {
      return res;
    }

    UBYTE *data = bu.buffer;
    if(data == 0) {
      return BOOTLOADER_RET_NO_PAGE_DATA;
    }

    /* set addr in bootloader */
    res = proto_lfunc_write(ph, BOOTLOADER_CHN_PAGES, bu.addr);
    if(res != PROTO_RET_OK) {
      return BOOTLOADER_RET_FAILED_SET_ADDR | res;
    }

    /* read flash page (and do flash) */
    res = proto_chn_msg_read(ph, BOOTLOADER_CHN_PAGES, data, page_words);
    if(res != PROTO_RET_OK) {
      return BOOTLOADER_RET_READ_PAGE_ERROR | res;
    }

    /* (optional) post func can verify read data */
    if(post_read_func != 0) {
      res = post_read_func(&bu, user_data);
      if(res != BOOTLOADER_RET_OK) {
        return res;
      }
    }

    bu.addr += page_size;
    data += page_size;
  }

  return BOOTLOADER_RET_OK;
}

int bootloader_leave(proto_env_handle_t *pb)
{
  int res;
  proto_handle_t *ph = proto_env_get_proto(pb);

  /* reset device */
  res = proto_reset(ph);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_NO_RESET | res;
  }

  /* make sure we are in the application */
  UWORD fw_id;
  res = proto_wfunc_read(ph, PROTO_WFUNC_READ_FW_ID, &fw_id);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  /* check bootloader version magic is NOT set */
  if(fw_id == FWID_BOOTLOADER_PABLO) {
    return BOOTLOADER_RET_NO_FIRMWARE;
  }

  return BOOTLOADER_RET_OK;
}

const char *bootloader_perror(int res)
{
  switch(res & BOOTLOADER_RET_MASK) {
    case BOOTLOADER_RET_OK:
      return "OK";
    case BOOTLOADER_RET_NO_RESET:
      return "no application detected";
    case BOOTLOADER_RET_NO_BOOTLOADER:
      return "no bootloader detected";
    case BOOTLOADER_RET_READ_ERROR:
      return "error reading read-only register";
    case BOOTLOADER_RET_NO_FIRMWARE:
      return "no firmware detected";
    case BOOTLOADER_RET_INVALID_FILE:
      return "invalid flash file";
    case BOOTLOADER_RET_WRONG_FILE_SIZE:
      return "wrong flash file size";
    case BOOTLOADER_RET_WRONG_FILE_MACHTAG:
      return "wrong flash file mach tag";
    case BOOTLOADER_RET_FAILED_SET_ADDR:
      return "failed to set page address";
    case BOOTLOADER_RET_WRITE_PAGE_ERROR:
      return "failed to write flash page";
    case BOOTLOADER_RET_READ_PAGE_ERROR:
      return "failed to read flash page";
    case BOOTLOADER_RET_NO_PAGE_DATA:
      return "no page data submitted";
    case BOOTLOADER_RET_DATA_MISMATCH:
      return "page data mismatch";
    default:
      return "?";
  }
}
