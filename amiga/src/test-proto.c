#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"
#include "pario.h"
#include "timer.h"
#include "proto.h"

int dosmain(void)
{
    struct pario_handle *ph;
    struct timer_handle *th;

    PutStr("test-proto\n");
    ph = pario_init((struct Library *)SysBase);
    if(ph != NULL) {
        struct pario_port *port = pario_get_port(ph);
        PutStr("timer_init\n");
        th = timer_init((struct Library *)SysBase);
        if(th != NULL) {
            /* setup proto low */
            PutStr("proto_init\n");
            struct proto_handle *ph = proto_init(port, th);
            if(ph != NULL) {
                PutStr("proto_ping\n");
                int error = proto_cmd(ph, PROTO_CMD_PING);
                Printf("-> %ld\n", (LONG)error);
                PutStr("done\n");
                proto_exit(ph);
            } else {
                PutStr("error setting up proto!\n");
            }
            timer_exit(th);
        } else {
            PutStr("error setting up timer!\n");
        }
        pario_exit(ph);
    } else {
        PutStr("error setting up pario!\n");
    }
    return 0;
}
