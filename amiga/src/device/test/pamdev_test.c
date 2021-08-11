#include <proto/exec.h>
#include <proto/dos.h>
#include <clib/alib_protos.h>

#include <dos/dos.h>
#include <exec/exec.h>

#include "autoconf.h"
#include "debug.h"

typedef struct IORequest IOR;

static const char *TEMPLATE = "D=DEVICE/K,U=UNIT/K/N";
typedef struct {
  char *device;
  ULONG *unit;
} params_t;
static params_t params;

int dosmain(void)
{
    struct RDArgs *args;
    char *device = "pambox.device";
    ULONG unit = 0;

    /* First parse args */
    args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
    if(args == NULL) {
        PrintFault(IoErr(), "Args Error");
        return RETURN_ERROR;
    }
    if(params.device) {
        device = params.device;
    }
    if(params.unit) {
        unit = *params.unit;
    }

    int result = 0;
    struct MsgPort *port = CreateMsgPort();
    if(port != NULL) {
        struct IOStdReq *req = CreateStdIO(port);
        if(req != NULL) {
            if (OpenDevice(device, unit, (IOR *)req, 0)) {
                Printf("Unable to open '%s':%ld, error %ld %ld\n",
                       (ULONG)device, unit, req->io_Error, req->io_Actual);
                result = 3;
            } else {
                PutStr("OK!\n");

                CloseDevice((IOR *)req);
                PutStr("done\n");
            }
            DeleteStdIO(req);
        } else {
            PutStr("can't create IO req!\n");
            result = 2;
        }
        DeleteMsgPort(port);
    } else {
        PutStr("can't create port!\n");
        result = 1;
    }

    /* Finally free args */
    FreeArgs(args);
    return result;
}
