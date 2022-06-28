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

        /* reset, load, save param */
        error = paloma_param_all_reset(pal);
        Printf("reset all params: %ld\n", error);

        error = paloma_param_all_load(pal);
        Printf("load all params: %ld\n", error);

        error = paloma_param_all_save(pal);
        Printf("save all param: %ld\n", error);

        /* get all slots */
        UBYTE num_slots = 0;
        error = paloma_param_get_total_slots(pal, &num_slots);
        Printf("num slots: %ld, %ld\n", error, (LONG)num_slots);

        /* get infos */
        for(UBYTE i=0; i<num_slots;i++) {
          paloma_param_info_t info;

          error = paloma_param_get_info(pal, i, &info);
          Printf("get info: %ld, %ld\n", (LONG)i, error);
          Printf("id=%02lx type=%ld max=%ld\n", (LONG)info.id, (LONG)info.type, (LONG)info.max_bytes);
          Printf("name=%s\n", &info.name);
        }

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
