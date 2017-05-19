#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"
#include "types.h"
#include "arch.h"

#include "parbox.h"
#include "pblfile.h"
#include "bootloader.h"
#include "machtag.h"

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

static void show_file_info(const char *file_name, pblfile_t *pf)
{
  Printf("PBL File:   size=%08lx, name='%s'\n", pf->rom_size, file_name);
  UBYTE bl_hi = (UBYTE)(pf->version >> 8);
  UBYTE bl_lo = (UBYTE)(pf->version & 0xff);
  char *arch,*mcu,*mach;
  UBYTE extra;
  machtag_decode(pf->mach_tag, &arch, &mcu, &mach, &extra);
  Printf("PBL File:   %ld.%ld, %s-%s-%s-%ld (%04lx)\n",
    (ULONG)bl_hi, (ULONG)bl_lo, arch, mcu, mach, (ULONG)extra, (ULONG)pf->mach_tag);
}

static void show_bootinfo(bootinfo_t *bi)
{
  Printf("Flash ROM:  size=%08lx, page=%04lx\n",
    bi->rom_size, (ULONG)bi->page_size);

  UBYTE bl_hi = (UBYTE)(bi->bl_version >> 8) & 0x7f;
  UBYTE bl_lo = (UBYTE)(bi->bl_version & 0xff);
  char *arch,*mcu,*mach;
  UBYTE extra;
  machtag_decode(bi->bl_mach_tag, &arch, &mcu, &mach, &extra);
  Printf("Bootloader: %ld.%ld, %s-%s-%s-%ld (%04lx)\n",
    (ULONG)bl_hi, (ULONG)bl_lo, arch, mcu, mach, (ULONG)extra, (ULONG)bi->bl_mach_tag);
}

static void show_fw_info(bootinfo_t *bi)
{
  if(bi->fw_mach_tag != bi->bl_mach_tag) {
    PutStr("Firmware:   not found.\n");
  } else {
    UBYTE fw_hi = (UBYTE)(bi->fw_version >> 8) & 0x7f;
    UBYTE fw_lo = (UBYTE)(bi->fw_version & 0xff);
    char *arch,*mcu,*mach;
    UBYTE extra;
    machtag_decode(bi->fw_mach_tag, &arch, &mcu, &mach, &extra);
    Printf("Firmware:   %ld.%ld, %s-%s-%s-%ld (%04lx)  crc=%04lx\n",
      (ULONG)fw_hi, (ULONG)fw_lo, arch, mcu, mach, (ULONG)extra, (ULONG)bi->fw_mach_tag,
      (ULONG)bi->fw_crc);
  }
}

static void show_error(int bl_res)
{
  PutStr("BL Error: ");
  PutStr(bootloader_perror(bl_res));
  PutStr(", (Proto=");
  PutStr(proto_perror(bl_res));
  PutStr(")\n");
}

static int flash_func(bl_flash_data_t *fd, void *user_data)
{
  pblfile_t *pf = (pblfile_t *)user_data;

  Printf("%08lx/%08lx\r", fd->addr, fd->max_addr);

  fd->buffer = pf->data + fd->addr;

  return BOOTLOADER_RET_OK;
}

static int do_flash(parbox_handle_t *pb, bootinfo_t *bi, pblfile_t *pf)
{
  int bl_res;

  PutStr("Flashing...\n");
  bl_res = bootloader_flash(pb, bi, flash_func, pf);
  if(bl_res == BOOTLOADER_RET_OK) {
    PutStr("\nDone\n");

    // after flashing re-read fw infos
    bl_res = bootloader_update_fw_info(pb, bi);
    if(bl_res == BOOTLOADER_RET_OK) {
      show_fw_info(bi);
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

static int do_verify(parbox_handle_t *pb, bootinfo_t *bi, pblfile_t *pf)
{
  int bl_res;

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

  bl_res = bootloader_read(pb, bi, pre_verify_func, post_verify_func, &md);
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
  parbox_handle_t pb;
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
        Printf("INVALID: %s\n", pblfile_perror(file_result));
        pblfile_free(&pf);
        res = RETURN_ERROR;
      }
    } else {
      /* error */
      Printf("FAILED loading '%s': %s\n",
        file_name, pblfile_perror(file_result));
      res = RETURN_ERROR;
    }
  }

  /* open parbox */
  if(file_result == PBLFILE_OK) {
    /* setup parbox */
    int pb_res = parbox_init(&pb, (struct Library *)SysBase);
    if(pb_res == PARBOX_OK) {
      bootinfo_t bi;

      PutStr("Entering bootloader...");
      int bl_res = bootloader_enter(&pb, &bi);
      if(bl_res == BOOTLOADER_RET_OK) {
        PutStr("ok\n");
        show_bootinfo(&bi);
        show_fw_info(&bi);

        // check a file
        if(file_name != 0) {
          PutStr("Matching flash file...");
          bl_res = bootloader_check_file(&bi, &pf);
          if(bl_res == BOOTLOADER_RET_OK) {
            PutStr("ok\n");
          } else {
            show_error(bl_res);
          }
        }

        // flash?
        if((bl_res == BOOTLOADER_RET_OK) && params.flash) {
          bl_res = do_flash(&pb, &bi, &pf);
        }

        // verify?
        if((bl_res == BOOTLOADER_RET_OK) && (params.verify || params.flash)) {
          bl_res = do_verify(&pb, &bi, &pf);
        }

        // leave bootloader? - only if no error occurred
        if(params.stay) {
          PutStr("Staying in bootloader as requested\n");
        }
        else if(bl_res == BOOTLOADER_RET_OK) {
          PutStr("Leaving bootloader...");
          bl_res = bootloader_leave(&pb);
          if(bl_res == BOOTLOADER_RET_OK) {
            PutStr("ok\n");
          } else {
            show_error(bl_res);
          }
        } else {
          PutStr("Staying in bootloader: due to errors!\n");
        }
      }
      else {
        show_error(bl_res);
      }

      parbox_exit(&pb);
    } else {
      Printf("FAILED parbox: %s\n", parbox_perror(pb_res));
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
