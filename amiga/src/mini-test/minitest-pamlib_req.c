#include <proto/exec.h>
#include <proto/dos.h>
#include <clib/alib_protos.h>

#include <dos/dos.h>
#include <exec/exec.h>

#include "autoconf.h"
#include "debug.h"

#include "pamlib.h"
#include "pamlib_req.h"

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

        PutStr("open cmd channel\n");
        pamlib_req_t *req = pamlib_req_open(ph, 2000, &error);
        Printf("-> %ld\n", error);

        UWORD mtu = pamlib_req_get_max_size(req);
        UBYTE *buf = AllocVec(mtu, MEMF_CLEAR);

        // handle request
        UWORD rep_size;
        error = pamlib_req_handle(req, buf, mtu, &rep_size);
        Printf("-> %ld, size %ld\n", error, (ULONG)rep_size);

        FreeVec(buf);

        PutStr("close cmd channel\n");
        error = pamlib_req_close(req);
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
