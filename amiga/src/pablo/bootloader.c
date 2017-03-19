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
  res = proto_cmd(ph, PROTO_CMD_PING);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_NO_PING | res;
  }

  /* try to enter bootloader (ignored if already running */
  res = proto_cmd(ph, PROTO_CMD_BOOTLOADER);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_NO_BOOTLOADER | res;
  }

  /* read version tag */
  UWORD bl_version;
  res = proto_reg_ro_read(ph, BOOTLOADER_RO_BL_VERSION, &bl_version);
  if(res != PROTO_RET_OK) {
    return BOOTLOADER_RET_REG_RO_ERROR | res;
  }

  bi->bl_version = bl_version;

  /* check bootloader version magic */
  if((bl_version & BOOTLOADER_VER_TAG) != BOOTLOADER_VER_TAG) {
    return BOOTLOADER_RET_NO_BOOTLOADER;
  }

  return BOOTLOADER_RET_OK;
}

int bootloader_leave(parbox_handle_t *pb, bootinfo_t *bi)
{
  return 0;
}

const char *bootloader_perror(int res)
{
  switch(res & BOOTLOADER_RET_MASK) {
    case BOOTLOADER_RET_OK:
      return "ok";
    default:
      return "?";
  }
}
