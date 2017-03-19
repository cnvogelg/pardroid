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
   "Info/S,"
   "Reset/S,"
   "FileName"
   ;
typedef struct {
  ULONG flash;
  ULONG verify;
  ULONG info;
  ULONG reset;
  char  *file_name;
} params_t;
static params_t params;

static int check_args(void)
{
  if(!params.flash && !params.verify && !params.info && !params.reset) {
    PutStr("No command given!\n");
    return 1;
  }

  if(params.flash || params.verify) {
    if(!params.file_name) {
      PutStr("No file given!\n");
      return 2;
    }
  }

  return 0;
}

static void show_bootinfo(bootinfo_t *bi)
{
  Printf("Flash ROM:  size=%08lx, page=%04lx\n",
    bi->rom_size, (ULONG)bi->page_size);

  UBYTE bl_hi = (UBYTE)(bi->bl_version >> 8) & 0x7f;
  UBYTE bl_lo = (UBYTE)(bi->bl_version & 0xff);
  char *arch,*mcu,*mach;
  machtag_decode(bi->bl_mach_tag, &arch, &mcu, &mach);
  Printf("Bootloader: %ld.%ld, %s-%s-%s (%04lx)\n",
    (ULONG)bl_hi, (ULONG)bl_lo, arch, mcu, mach, (ULONG)bi->bl_mach_tag);
}

static void show_fw_info(bootinfo_t *bi)
{
  if(bi->fw_mach_tag != bi->bl_mach_tag) {
    PutStr("Firmware:   not found.\n");
  } else {
    UBYTE fw_hi = (UBYTE)(bi->fw_version >> 8) & 0x7f;
    UBYTE fw_lo = (UBYTE)(bi->fw_version & 0xff);
    char *arch,*mcu,*mach;
    machtag_decode(bi->fw_mach_tag, &arch, &mcu, &mach);
    Printf("Firmware:   %ld.%ld, %s-%s-%s (%04lx)  crc=%04lx\n",
      (ULONG)fw_hi, (ULONG)fw_lo, arch, mcu, mach, (ULONG)bi->fw_mach_tag,
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

static void update_cb_func(bl_update_t *bu)
{
  Printf("%08lx: %ld/%ld\r", bu->addr, bu->cur_page, bu->num_pages);
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
      Printf("Loaded '%s':\n  size=%06lx, version=%04lx, mach_tag=%04lx\n",
        file_name, pf.rom_size, (ULONG)pf.version, (ULONG)pf.mach_tag);

      /* check data */
      file_result = pblfile_check(&pf);
      if(file_result == PBLFILE_OK) {
        PutStr("Data: ok.\n");
      } else {
        Printf("Data: INVALID: %s\n", pblfile_perror(file_result));
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

        // flash?
        if(params.flash) {
          PutStr("Flashing...\n");
          bl_res = bootloader_flash(&pb, &bi, &pf, update_cb_func);
          if(bl_res == BOOTLOADER_RET_OK) {
            PutStr("\nDone\n");

            // after flashing re-read fw infos
            bl_res = bootloader_update_fw_info(&pb, &bi);
            if(bl_res == BOOTLOADER_RET_OK) {
              show_fw_info(&bi);
            } else {
              PutStr("Read Info Aborted: ");
              show_error(bl_res);
            }
          } else {
            PutStr("\nAborted: ");
            show_error(bl_res);
          }
        }

        // verify?
        if((bl_res == BOOTLOADER_RET_OK) && (params.verify || params.flash)) {
          PutStr("Verifying...");
          bl_res = bootloader_verify(&pb, &bi, &pf, update_cb_func);
          if(bl_res == BOOTLOADER_RET_OK) {
            PutStr("\nDone\n");
          } else {
            PutStr("\nAborted: ");
            show_error(bl_res);
          }
        }

        // leave bootloader? - only if no error occurred
        if(bl_res == BOOTLOADER_RET_OK) {
          PutStr("Leaving bootloader...");
          bl_res = bootloader_leave(&pb);
          if(bl_res == BOOTLOADER_RET_OK) {
            PutStr("ok\n");
          } else {
            show_error(bl_res);
          }
        } else {
          PutStr("Staying in bootloader due to errors!\n");
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
