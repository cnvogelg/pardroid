#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"
#include "types.h"
#include "arch.h"

#include "proto_env.h"
#include "pblfile.h"
#include "bootloader.h"
#include "machtag.h"
#include "fwid.h"

static const char *TEMPLATE =
   "Flash/S,"
   "Verify/S,"
   "Stay/S,"
   "FileName"
   ;
typedef struct {
  ULONG flash;
  ULONG verify;
  ULONG stay;
  char  *file_name;
} params_t;
static params_t params;

static int check_args(void)
{
  if(params.flash || params.verify) {
    if(!params.file_name) {
      PutStr("No file given!\n");
      return 2;
    }
  }

  return 0;
}

static void show_machtag(UWORD mach_tag)
{
  const char *arch,*mcu,*mach;
  UBYTE extra;
  machtag_decode(mach_tag, &arch, &mcu, &mach, &extra);
  Printf("            machtag=%s-%s-%s-%ld (%04lx)\n",
     (ULONG)arch, (ULONG)mcu, (ULONG)mach, (ULONG)extra, (ULONG)mach_tag);
}

static void show_file_info(const char *file_name, pblfile_t *pf)
{
  Printf("PBL File:   size=%08lx, name='%s'\n", pf->rom_size, (ULONG)file_name);
  UBYTE bl_hi = (UBYTE)(pf->version >> 8);
  UBYTE bl_lo = (UBYTE)(pf->version & 0xff);
  const char *id_str;
  fwid_decode(pf->fw_id, &id_str);
  Printf("            fw=%04lx (%s), ver=%ld.%ld\n",
    (ULONG)pf->fw_id, (ULONG)id_str, (ULONG)bl_hi, (ULONG)bl_lo);
  show_machtag(pf->mach_tag);
}

static void show_bootinfo(boot_handle_t *bh)
{
  bootinfo_t *bi = &bh->info;

  Printf("Flash ROM:  size=%08lx, page=%04lx\n",
    bi->rom_size, (ULONG)bi->page_size);

  UBYTE bl_hi = (UBYTE)(bi->bl_version >> 8) & 0x7f;
  UBYTE bl_lo = (UBYTE)(bi->bl_version & 0xff);
  Printf("Bootloader: ver=%ld.%ld\n",
    (ULONG)bl_hi, (ULONG)bl_lo);
  show_machtag(bi->bl_mach_tag);
}

static void show_fw_info(boot_handle_t *bh)
{
  bootinfo_t *bi = &bh->info;

  if(bi->fw_mach_tag != bi->bl_mach_tag) {
    PutStr("Firmware:   not found.\n");
  } else {
    UBYTE fw_hi = (UBYTE)(bi->fw_version >> 8) & 0x7f;
    UBYTE fw_lo = (UBYTE)(bi->fw_version & 0xff);
    const char *arch,*mcu,*mach;
    UBYTE extra;
    machtag_decode(bi->fw_mach_tag, &arch, &mcu, &mach, &extra);
    const char *id_str;
    fwid_decode(bi->fw_id, &id_str);
    Printf("Firmware:   fw=%04lx (%s), ver=%ld.%ld, crc=%04lx\n",
      (ULONG)bi->fw_id, (ULONG)id_str, (ULONG)fw_hi, (ULONG)fw_lo,
      (ULONG)bi->fw_crc);
    show_machtag(bi->fw_mach_tag);
  }
}

static void show_error(int bl_res)
{
  PutStr("BL Error: ");
  PutStr(bootloader_perror(bl_res));
  PutStr(", (Proto=");
  PutStr(proto_atom_perror(bl_res & BOOTLOADER_RET_PROTO_MASK));
  PutStr(")\n");
}

static int flash_func(bl_flash_data_t *fd, void *user_data)
{
  pblfile_t *pf = (pblfile_t *)user_data;

  Printf("%08lx/%08lx\r", fd->addr, fd->max_addr);

  fd->buffer = pf->data + fd->addr;

  return BOOTLOADER_RET_OK;
}

static int do_flash(boot_handle_t *bh, pblfile_t *pf)
{
  int bl_res;

  PutStr("Flashing...\n");
  bl_res = bootloader_flash(bh, flash_func, pf);
  if(bl_res == BOOTLOADER_RET_OK) {
    PutStr("\nDone\n");

    // after flashing re-read fw infos
    bl_res = bootloader_update_fw_info(bh);
    if(bl_res == BOOTLOADER_RET_OK) {
      show_fw_info(bh);
    } else {
      PutStr("Read Info Aborted: ");
      show_error(bl_res);
    }
  } else {
    PutStr("\nAborted: ");
    show_error(bl_res);
  }

  return bl_res;
}

struct my_verify_data {
  pblfile_t  *pf;
  UBYTE      *page_buf;
  UWORD       page_size;
};

static int pre_verify_func(bl_flash_data_t *fd, void *user_data)
{
  struct my_verify_data *md = (struct my_verify_data *)user_data;

  Printf("%08lx/%08lx\r", fd->addr, fd->max_addr);

  /* always read into page buffer */
  fd->buffer = md->page_buf;

  return BOOTLOADER_RET_OK;
}

static int post_verify_func(bl_flash_data_t *fd, void *user_data)
{
  struct my_verify_data *md = (struct my_verify_data *)user_data;

  /* compare page buffer with pblfile */
  UBYTE *file_buf = md->pf->data + fd->addr;
  UBYTE *page_buf = md->page_buf;
  int failed = 0;
  for(UWORD i=0;i<md->page_size;i++) {
    if(file_buf[i] != page_buf[i]) {
      Printf("@%08lx: %02lx != %02lx\n",
          fd->addr+i, (ULONG)file_buf[i], (ULONG)page_buf[i]);
      failed++;
    }
  }

  if(failed == 0) {
    return BOOTLOADER_RET_OK;
  } else {
    return BOOTLOADER_RET_DATA_MISMATCH;
  }
}

static int do_verify(boot_handle_t *bh, pblfile_t *pf)
{
  int bl_res;
  bootinfo_t *bi = &bh->info;

  PutStr("Verifying...");

  /* alloc page buffer */
  UBYTE *page_buf = AllocVec(bi->page_size, MEMF_CLEAR | MEMF_PUBLIC);
  if(page_buf == 0) {
    return BOOTLOADER_RET_NO_PAGE_DATA;
  }

  struct my_verify_data md;
  md.pf = pf;
  md.page_buf = page_buf;
  md.page_size = bi->page_size;

  bl_res = bootloader_read(bh, pre_verify_func, post_verify_func, &md);
  if(bl_res == BOOTLOADER_RET_OK) {
    PutStr("\nDone\n");
  } else {
    PutStr("\nAborted: ");
    show_error(bl_res);
  }

  /* free page buffer */
  FreeVec(page_buf);

  return bl_res;
}

int dosmain(void)
{
  struct RDArgs *args;
  proto_env_handle_t *pb;
  pblfile_t pf;

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if(args == NULL) {
    PutStr(TEMPLATE);
    PutStr("  Invalid Args!\n");
    return RETURN_ERROR;
  }
  /* check args */
  if(check_args()) {
    return RETURN_ERROR;
  }

  int res = RETURN_OK;

  /* load file? */
  int file_result = PBLFILE_OK;
  char *file_name = params.file_name;
  if(file_name != 0) {
    file_result = pblfile_load(file_name, &pf);
    if(file_result == PBLFILE_OK) {
      show_file_info(file_name, &pf);

      /* check data */
      PutStr("Checking file contents...");
      file_result = pblfile_check(&pf);
      if(file_result == PBLFILE_OK) {
        PutStr("ok.\n");
      } else {
        Printf("INVALID: %s\n", (ULONG)pblfile_perror(file_result));
        pblfile_free(&pf);
        res = RETURN_ERROR;
      }
    } else {
      /* error */
      Printf("FAILED loading '%s': %s\n",
        (ULONG)file_name, (ULONG)pblfile_perror(file_result));
      res = RETURN_ERROR;
    }
  }

  /* open pamela */
  if(file_result == PBLFILE_OK) {
    /* setup pamela */
    int pb_res;
    pb = proto_env_init((struct Library *)SysBase, &pb_res);
    if(pb_res == PROTO_ENV_OK) {
      boot_handle_t bh;

      PutStr("Entering bootloader...");
      int bl_res = bootloader_init(pb, &bh);
      if(bl_res == BOOTLOADER_RET_OK) {
        PutStr("ok\n");
        show_bootinfo(&bh);
        show_fw_info(&bh);

        // check a file
        if(file_name != 0) {
          PutStr("Matching flash file...");
          bl_res = bootloader_check_file(&bh, &pf);
          if(bl_res == BOOTLOADER_RET_OK) {
            PutStr("ok\n");
          } else {
            show_error(bl_res);
          }
        }

        // flash?
        if((bl_res == BOOTLOADER_RET_OK) && params.flash) {
          bl_res = do_flash(&bh, &pf);
        }

        // verify?
        if((bl_res == BOOTLOADER_RET_OK) && (params.verify || params.flash)) {
          bl_res = do_verify(&bh, &pf);
        }

        // leave bootloader? - only if no error occurred
        if(params.stay) {
          PutStr("Staying in bootloader as requested\n");
        }
        else if(bl_res == BOOTLOADER_RET_OK) {
          PutStr("Leaving bootloader...");
          bl_res = bootloader_leave(&bh);
          if(bl_res == BOOTLOADER_RET_OK) {
            PutStr("ok\n");
          } else {
            show_error(bl_res);
          }
        } else {
          PutStr("Staying in bootloader: due to errors!\n");
        }

        // free bootloader res
        bootloader_exit(&bh);
      }
      else {
        show_error(bl_res);
      }

      proto_env_exit(pb);
    } else {
      Printf("FAILED proto env setup: %s\n", (LONG)proto_env_perror(pb_res));
      res = RETURN_ERROR;
    }

    /* free file? */
    if(params.file_name != 0) {
      pblfile_free(&pf);
    }
  }

  /* Finally free args */
  FreeArgs(args);
  return res;
}
