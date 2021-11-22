#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"
#include "proto_dev.h"

int dosmain(void)
{
    proto_env_handle_t *penv;
    int error;

    PutStr("test-proto-dev\n");
    penv = proto_env_init((struct Library *)SysBase, &error);
    if(penv != NULL) {
        PutStr("proto env OK\n");

        /* setup proto low */
        PutStr("proto_dev_init\n");
        proto_handle_t *ph = proto_dev_init(penv);
        if(ph != NULL) {
            Printf("proto %ld\n", ph);

            int res = proto_dev_action_ping(ph);
            Printf("ping: ret=%ld\n", res);

            proto_dev_exit(ph);
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
