#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "parbox.h"
#include "pblfile.h"

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
      PutStr("Welcome to pablo!\n");

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
