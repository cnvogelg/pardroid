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

    PutStr("test-proto-atom\n");
    penv = proto_env_init((struct Library *)SysBase, &error);
    if(penv != NULL) {
        PutStr("proto env OK\n");

        /* setup proto low */
        PutStr("proto_dev_init\n");
        proto_handle_t *ph = proto_devinit(penv);
        if(ph != NULL) {
            Printf("proto %ld\n", ph);



            proto_dev_exit(ph);
            PutStr("proto done\n");
        } else {
            PutStr("error setting up proto!\n");
        }
        proto_env_exit(penv);
        PutStr("penv done\n");
    } else {
        Printf("error setting up pario!\n");
    }
    PutStr("done\n");
    return 0;
}
