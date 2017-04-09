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
        time_stamp_t e1;
        time_stamp_t e2;
        ULONG ef = timer_get_eclock(th, &e1);
        Delay(100);
        timer_get_eclock(th, &e2);
        Printf("efreq: %ld\n", ef);
        Printf("eclock: e1=%08lx.%08lx e2=%08lx.%08lx\n", e1.hi, e1.lo, e2.hi, e2.lo);
        timer_delta(th, &e2, &e1);
        Printf("eclock: diff=%08lx.%08lx\n", e2.hi, e2.lo);
        ULONG us = timer_eclock_to_us(th, &e2);
        Printf("us: %ld\n", us);
        ULONG bps = timer_calc_bps(th, &e2, 500UL);
        Printf("bps: %ld\n", bps);

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
