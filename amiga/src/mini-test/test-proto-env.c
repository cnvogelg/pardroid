#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"
#include "pario.h"
#include "timer.h"
#include "proto_env.h"

int dosmain(void)
{
    struct proto_env_handle *penv;
    int error;

    PutStr("test-proto-env\n");
    penv = proto_env_init((struct Library *)SysBase, &error);
    if(penv != NULL) {
        PutStr("proto env OK\n");
        proto_env_exit(penv);
        PutStr("done\n");
    } else {
        Printf("proto env FAILED: %ld\n", error);
    }
    return 0;
}
