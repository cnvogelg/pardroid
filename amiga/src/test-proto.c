#include <proto/exec.h>
#include <dos/dos.h>

#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"
#include "pario.h"
#include "timer.h"
#include "proto.h"

int main(int argc, char **argv)
{
    struct pario_handle *ph;
    struct timer_handle *th;

    puts("test-proto");
    ph = pario_init((struct Library *)SysBase);
    if(ph != NULL) {
        struct pario_port *port = pario_get_port(ph);
        puts("timer_init");
        th = timer_init((struct Library *)SysBase);
        if(th != NULL) {
            /* setup proto low */
            puts("proto_init");
            struct proto_handle *ph = proto_init(port, th);
            if(ph != NULL) {
                puts("proto_ping");
                int error = proto_ping(ph);
                printf("-> %d\n", error);
                puts("done");
                proto_exit(ph);
            } else {
                puts("error setting up proto!");
            }
            timer_exit(th);
        } else {
            puts("error setting up timer!");
        }
        pario_exit(ph);
    } else {
        puts("error setting up pario!");
    }
    return 0;
}
