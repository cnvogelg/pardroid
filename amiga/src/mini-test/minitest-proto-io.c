#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"
#include "proto_io.h"

int dosmain(void)
{
    proto_env_handle_t *penv;
    int error;

    PutStr("test-proto-io\n");
    penv = proto_env_init((struct Library *)SysBase, &error);
    if(penv != NULL) {
        PutStr("proto env OK\n");

        /* setup proto low */
        PutStr("proto_io_init\n");
        proto_handle_t *ph = proto_io_init(penv);
        if(ph != NULL) {
            Printf("proto %ld\n", (ULONG)ph);

            UWORD mask = 0;
            int res = proto_io_get_event_mask(ph, &mask);
            Printf("event mask: ret=%ld mask=%x\n", res, mask);

            proto_io_exit(ph);
            PutStr("proto done\n");
        } else {
            PutStr("error setting up proto!\n");
        }
        proto_env_exit(penv);
        PutStr("penv done\n");
    } else {
        Printf("error setting up proto env!\n");
    }
    PutStr("done\n");
    return 0;
}
