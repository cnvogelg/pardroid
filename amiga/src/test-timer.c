#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "debug.h"
#include "timer.h"

int dosmain(void)
{
    struct timer_handle *th;

    PutStr("test-timer\n");
    th = timer_init((struct Library *)SysBase);
    if(th != NULL) {
        PutStr("got timer\n");

        /* test how fast eclock reads are */
        ULONG e1 = timer_get_eclock(th);
        ULONG e2 = timer_get_eclock(th);
        Printf("eclock: e1=%08lx e2=%08lx\n", e1, e2);

        /* test timer: busy wait for timeout */
        volatile UBYTE *flag = timer_get_flag(th);
        PutStr("start timer\n");
        timer_start(th, 0, 100000UL);
        ULONG i = 0;
        while(!*flag) {
            i++;
        }
        Printf("dong: %lu\n", i);

        /* test stop timer */
        PutStr("start timer\n");
        timer_start(th, 0, 100000UL);
        PutStr("stop timer\n");
        timer_stop(th);

        PutStr("start again\n");
        timer_start(th, 0, 100000UL);
        i = 0;
        while(!*flag) {
            i++;
        }
        Printf("dong: %lu\n", i);

        timer_exit(th);
        PutStr("done\n");
    } else {
        PutStr("no timer\n");
    }
    return 0;
}
