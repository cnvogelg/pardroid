#include <proto/exec.h>
#include <proto/dos.h>
#include <clib/alib_protos.h>

#include <dos/dos.h>
#include <exec/exec.h>

#include "autoconf.h"
#include "debug.h"

#include "pamlib.h"
#include "paloma_lib.h"
#include "parbox/ports.h"

static const char *TEMPLATE = "D=DEVICE/K";
typedef struct {
  char *device;
} params_t;
static params_t params;

int dosmain(void)
{
    struct RDArgs *args;
    char *device = "pamela.device";
    pamlib_handle_t *pam;
    paloma_handle_t *pal;

    /* First parse args */
    args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
    if(args == NULL) {
        PrintFault(IoErr(), "Args Error");
        return RETURN_ERROR;
    }
    if(params.device) {
        device = params.device;
    }

    int result = RETURN_OK;
    int error = 0;

    /* open pamlib */
    pam = pamlib_init((struct Library *)SysBase, &error, device);
    if(pam != NULL) {

      /* open paloma */
      pal = paloma_init((struct Library *)SysBase, pam, PALOMA_DEFAULT_PORT, &error);
      if(pal != NULL) {
        PutStr("paloma OK\n");

        /* close paloma */
        paloma_exit(pal);
      } else {
        Printf("can't init paloma: %ld %s\n", error, (LONG)pamela_perror(error));
        result = RETURN_ERROR;
      }

      /* close pamlib */
      pamlib_exit(pam);
    } else {
        Printf("can't init pamlib: %ld %s\n", error, (LONG)pamela_perror(error));
        result = RETURN_ERROR;
    }

    /* Finally free args */
    FreeArgs(args);
    return result;
}
