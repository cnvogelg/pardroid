#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "bootloader.h"
#include "proto.h"

int bootloader_enter(parbox_handle_t *pb, bootinfo_t *bi)
{
  int res;
  proto_handle_t *ph = pb->proto;

  /* first ping device */
  res = proto_action(ph, PROTO_ACTION_PING);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_NO_PING | res;
  }

  /* try to enter bootloader (ignored if already running */
  res = proto_action(ph, PROTO_ACTION_BOOTLOADER);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_NO_BOOTLOADER | res;
  }

  /* read version tag */
  UWORD bl_version;
  res = proto_reg_read(ph, BOOTLOADER_REG_BL_VERSION, &bl_version);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_REG_RO_ERROR | res;
  }

  bi->bl_version = bl_version;

  /* check bootloader version magic */
  if((bl_version & BOOTLOADER_VER_TAG) != BOOTLOADER_VER_TAG) {
    return BOOTLOADER_RET_NO_BOOTLOADER;
  }

  /* bootloader mach tag */
  res = proto_reg_read(ph, BOOTLOADER_REG_BL_MACHTAG, &bi->bl_mach_tag);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_REG_RO_ERROR | res;
  }

  /* page size */
  res = proto_reg_read(ph, BOOTLOADER_REG_PAGE_SIZE, &bi->page_size);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_REG_RO_ERROR | res;
  }

  /* rom size */
  UWORD size;
  res = proto_reg_read(ph, BOOTLOADER_REG_ROM_SIZE, &size);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_REG_RO_ERROR | res;
  }
  bi->rom_size = size;

  return bootloader_update_fw_info(pb, bi);
}

int bootloader_update_fw_info(parbox_handle_t *pb, bootinfo_t *bi)
{
  int res;
  proto_handle_t *ph = pb->proto;

  /* firmware crc */
  res = proto_reg_read(ph, BOOTLOADER_REG_FW_CRC, &bi->fw_crc);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_REG_RO_ERROR | res;
  }

  /* firmware mach tag */
  res = proto_reg_read(ph, BOOTLOADER_REG_FW_MACHTAG, &bi->fw_mach_tag);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_REG_RO_ERROR | res;
  }

  /* firmware version */
  res = proto_reg_read(ph, BOOTLOADER_REG_FW_VERSION, &bi->fw_version);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_REG_RO_ERROR | res;
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

int bootloader_flash(parbox_handle_t *pb, bootinfo_t *bi,
                     bl_flash_cb_t pre_flash_func,
                     void *user_data)
{
  int res;
  proto_handle_t *ph = pb->proto;

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
    UWORD addr = bu.addr;
    res = proto_reg_write(ph, BOOTLOADER_REG_PAGE_ADDR, addr);
    if(res != PROTO_RET_OK) {
      return BOOTLOADER_RET_FAILED_SET_ADDR | res;
    }

    /* send flash page (and do flash) */
    res = proto_msg_write_single(ph, BOOTLOADER_CHN_PAGES, data, page_words);
    if(res != PROTO_RET_OK) {
      return BOOTLOADER_RET_WRITE_PAGE_ERROR | res;
    }

    bu.addr += page_size;
    data += page_size;
  }

  return BOOTLOADER_RET_OK;
}

int bootloader_read(parbox_handle_t *pb, bootinfo_t *bi,
                    bl_read_cb_t pre_read_func,
                    bl_read_cb_t post_read_func,
                    void *user_data)
{
  int res;
  proto_handle_t *ph = pb->proto;

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
    UWORD addr = bu.addr;
    res = proto_reg_write(ph, BOOTLOADER_REG_PAGE_ADDR, addr);
    if(res != PROTO_RET_OK) {
      return BOOTLOADER_RET_FAILED_SET_ADDR | res;
    }

    /* read flash page (and do flash) */
    UWORD size = page_words;
    res = proto_msg_read_single(ph, BOOTLOADER_CHN_PAGES, data, &size);
    if(res != PROTO_RET_OK) {
      return BOOTLOADER_RET_READ_PAGE_ERROR | res;
    }

    /* check size */
    if(size != page_words) {
      return BOOTLOADER_RET_READ_PAGE_ERROR;
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

int bootloader_leave(parbox_handle_t *pb)
{
  int res;
  proto_handle_t *ph = pb->proto;

  /* reset device */
  res = proto_action(ph, PROTO_ACTION_RESET);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_NO_BOOTLOADER | res;
  }

  /* try multiple times to ping the device */
  for(int i=0;i<5;i++) {
    /* ping device */
    res = proto_action(ph, PROTO_ACTION_PING);
    if(res == PROTO_RET_OK) {
      break;
    }
  }
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_NO_PING | res;
  }

  /* read version tag from running firmware */
  UWORD bl_version;
  res = proto_reg_read(ph, BOOTLOADER_REG_BL_VERSION, &bl_version);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_REG_RO_ERROR | res;
  }

  /* check bootloader version magic is NOT set */
  if((bl_version & BOOTLOADER_VER_TAG) != 0) {
    return BOOTLOADER_RET_NO_FIRMWARE;
  }

  return BOOTLOADER_RET_OK;
}

const char *bootloader_perror(int res)
{
  switch(res & BOOTLOADER_RET_MASK) {
    case BOOTLOADER_RET_OK:
      return "OK";
    case BOOTLOADER_RET_NO_PING:
      return "no ping!";
    case BOOTLOADER_RET_NO_BOOTLOADER:
      return "no bootloader detected";
    case BOOTLOADER_RET_REG_RO_ERROR:
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
