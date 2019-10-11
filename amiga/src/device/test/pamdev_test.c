#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/alib.h>

#include <dos/dos.h>
#include <exec/exec.h>

#include "autoconf.h"
#include "debug.h"

typedef struct IORequest IOR;

int dosmain(void)
{
    int result = 0;
    struct MsgPort *port = CreateMsgPort();
    if(port != NULL) {
        struct IOStdReq *req = CreateStdIO(port);
        if(req != NULL) {
            if (OpenDevice("pambox.device", 0, (IOR *)req, 0)) {
                Printf("Unable to open pambox.device, error %ld %ld\n",
                       req->io_Error, req->io_Actual);
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
    return result;
}
