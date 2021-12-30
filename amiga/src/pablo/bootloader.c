#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "bootloader.h"
#include "proto_boot.h"
#include "proto_dev.h"
#include "types.h"
#include "arch.h"
#include "fwid.h"

int bootloader_init(proto_env_handle_t *penv, boot_handle_t *bh)
{
  int res;
  bootinfo_t *bi = &bh->info;
 
  /* enter boot loader */
  bh->proto = proto_boot_init(penv);
  if(bh->proto == NULL) {
    return BOOTLOADER_RET_NO_BOOTLOADER | res;
  }
  proto_handle_t *ph = bh->proto;

  /* check bootloader magic */
  UWORD fw_id;
  res = proto_dev_get_fw_id(ph, &fw_id);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }
  if(fw_id != FWID_BOOTLOADER_PABLO) {
    return BOOTLOADER_RET_NO_BOOTLOADER_MAGIC;
  }

  /* read version tag */
  UWORD bl_version;
  res = proto_dev_get_fw_version(ph, &bl_version);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  bi->bl_version = bl_version;

  /* bootloader mach tag */
  res = proto_dev_get_mach_tag(ph, &bi->bl_mach_tag);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  /* page size in bytes */
  res = proto_boot_get_page_size(ph, &bi->page_size);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  /* rom size in bytes */
  res = proto_boot_get_rom_size(ph, &bi->rom_size);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  return bootloader_update_fw_info(bh);
}

int bootloader_update_fw_info(boot_handle_t *bh)
{
  int res;
  proto_handle_t *ph = bh->proto;
  bootinfo_t *bi = &bh->info;

  /* firmware crc */
  res = proto_boot_get_rom_crc(ph, &bi->fw_crc);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  /* firmware mach tag */
  res = proto_boot_get_rom_mach_tag(ph, &bi->fw_mach_tag);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  /* firmware version */
  res = proto_boot_get_rom_fw_version(ph, &bi->fw_version);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  /* firmware id */
  res = proto_boot_get_rom_fw_id(ph, &bi->fw_id);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_READ_ERROR | res;
  }

  return BOOTLOADER_RET_OK;
}

int bootloader_check_file(boot_handle_t *bh, pblfile_t *pf)
{
  bootinfo_t *bi = &bh->info;

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

int bootloader_flash(boot_handle_t *bh,
                     bl_flash_cb_t pre_flash_func,
                     void *user_data)
{
  int res;
  proto_handle_t *ph = bh->proto;
  bootinfo_t *bi = &bh->info;

  UWORD page_size = bi->page_size;
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
    res = proto_boot_set_page_addr(ph, bu.addr);
    if(res != PROTO_RET_OK) {
      return BOOTLOADER_RET_FAILED_SET_ADDR | res;
    }

    /* send flash page (and do flash) */
    res = proto_boot_write_page(ph, data, page_size);
    if(res != PROTO_RET_OK) {
      return BOOTLOADER_RET_WRITE_PAGE_ERROR | res;
    }

    bu.addr += page_size;
    data += page_size;
  }

  return BOOTLOADER_RET_OK;
}

int bootloader_read(boot_handle_t *bh,
                    bl_read_cb_t pre_read_func,
                    bl_read_cb_t post_read_func,
                    void *user_data)
{
  int res;
  proto_handle_t *ph = bh->proto;
  bootinfo_t *bi = &bh->info;

  UWORD page_size = bi->page_size;
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
    res = proto_boot_set_page_addr(ph, bu.addr);
    if(res != PROTO_RET_OK) {
      return BOOTLOADER_RET_FAILED_SET_ADDR | res;
    }

    /* read flash page (and do flash) */
    res = proto_boot_read_page(ph, data, page_size);
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

int bootloader_leave(boot_handle_t *bh)
{
  int res, retval;
  proto_handle_t *ph = bh->proto;

  /* leave boot loader */
  res = proto_boot_leave(ph);
  if(res != PROTO_RET_OK) {
    retval = BOOTLOADER_RET_NO_RESET | res;
  }

  /* make sure we are in the application */
  UWORD fw_id;
  res = proto_dev_get_fw_id(ph, &fw_id);
  if(res != PROTO_RET_OK) {
    retval = BOOTLOADER_RET_READ_ERROR | res;
  }

  /* check bootloader version magic is NOT set */
  if(fw_id == FWID_BOOTLOADER_PABLO) {
    retval = BOOTLOADER_RET_NO_FIRMWARE;
  } else {
    retval = BOOTLOADER_RET_OK;
  }

  return retval;
}

void bootloader_exit(boot_handle_t *bh)
{
  proto_handle_t *ph = bh->proto;

  proto_boot_exit(ph);
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
