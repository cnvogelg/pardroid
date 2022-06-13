#include <proto/exec.h>
#include <proto/dos.h>
#include <clib/alib_protos.h>

#include <dos/dos.h>
#include <exec/exec.h>

#include "autoconf.h"
#include "debug.h"

#include "pamlib.h"

static const char *TEMPLATE = "D=DEVICE/K";
typedef struct {
  char *device;
} params_t;
static params_t params;

int dosmain(void)
{
    struct RDArgs *args;
    char *device = "pamela.device";
    pamlib_handle_t *ph;

    /* First parse args */
    args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
    if(args == NULL) {
        PrintFault(IoErr(), "Args Error");
        return RETURN_ERROR;
    }
    if(params.device) {
        device = params.device;
    }

    int result = 0;
    int error = 0;
    ph = pamlib_init((struct Library *)SysBase, &error, device);
    if(ph != NULL) {

        pamela_devinfo_t dev_info;
        PutStr("get devinfo\n");
        error = pamlib_devinfo(ph, &dev_info);
        Printf("-> %ld\n", error);

        PutStr("open channel\n");
        pamlib_channel_t *ch = pamlib_open(ph, 1234, &error);
        Printf("-> %ld\n", error);

        PutStr("write\n");
        error = pamlib_write(ch, (BYTE *)&dev_info, sizeof(dev_info));
        Printf("-> %ld\n", error);

        PutStr("read\n");
        error = pamlib_read(ch, (BYTE *)&dev_info, sizeof(dev_info));
        Printf("-> %ld\n", error);

        PutStr("seek\n");
        error = pamlib_seek(ch, 0x12345678);
        Printf("-> %ld\n", error);

        PutStr("tell\n");
        ULONG pos = 0;
        error = pamlib_tell(ch, &pos);
        Printf("-> %ld, %ld\n", error, pos);

        PutStr("close channel\n");
        error = pamlib_close(ch);
        Printf("-> %ld\n", error);

        PutStr("exit pamlib\n");
        pamlib_exit(ph);
        PutStr("done\n");

    } else {
        Printf("can't init pamlib: %ld\n", error);
        result = 1;
    }

    /* Finally free args */
    FreeArgs(args);
    return result;
}
