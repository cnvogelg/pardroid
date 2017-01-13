#include <proto/exec.h>
#include <dos/dos.h>

#include <stdio.h>

#include "autoconf.h"
#include "debug.h"
#include "timer.h"

int main(int argc, char **argv)
{
    struct timer_handle *th;

    puts("test-timer");
    th = timer_init((struct Library *)SysBase);
    if(th != NULL) {
        puts("got timer");

        /* test how fast eclock reads are */
        ULONG e1 = timer_get_eclock(th);
        ULONG e2 = timer_get_eclock(th);
        printf("eclock: e1=%08lx e2=%08lx\n", e1, e2);

        /* test timer: busy wait for timeout */
        volatile UBYTE *flag = timer_get_flag(th);
        puts("start timer");
        timer_start(th, 0, 100000UL);
        ULONG i = 0;
        while(!*flag) {
            i++;
        }
        printf("dong: %lu\n", i);

        /* test stop timer */
        puts("start timer");
        timer_start(th, 0, 100000UL);
        puts("stop timer");
        timer_stop(th);

        puts("start again");
        timer_start(th, 0, 100000UL);
        i = 0;
        while(!*flag) {
            i++;
        }
        printf("dong: %lu\n", i);

        timer_exit(th);
        puts("done");
    } else {
        puts("no timer");
    }
    return 0;
}
