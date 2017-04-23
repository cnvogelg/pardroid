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
        ULONG ef = timer_eclock_get(th, &e1);
        Delay(100);
        timer_eclock_get(th, &e2);
        Printf("efreq: %ld\n", ef);

        ULONG hi1, lo1, hi2, lo2;
        timer_eclock_split(&e1, &hi1, &lo1);
        timer_eclock_split(&e2, &hi2, &lo2);
        Printf("eclock: e1=%08lx.%08lx e2=%08lx.%08lx\n", hi1, lo1, hi2, lo2);

        time_stamp_t delta;
        timer_eclock_delta(&e2, &e1, &delta);
        ULONG hid, lod;
        timer_eclock_split(&delta, &hid, &lod);
        Printf("eclock: diff=%08lx.%08lx\n", hid, lod);
        ULONG us = timer_eclock_to_us(th, &delta);
        Printf("us: %ld\n", us);
        ULONG bps = timer_eclock_to_bps(th, &delta, 100UL);
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

        /* test sig timer */
        BYTE signal = timer_sig_init(th);
        if(signal != -1) {
            ULONG sigmask = 1 << signal;
            Printf("sig timer: sigmask=%08lx\n", sigmask);

            timer_sig_start(th, 0, 1000000UL);
            Wait(sigmask);
            PutStr("got timer");

            timer_sig_exit(th);
        } else {
            PutStr("No sig timer!!\n");
        }

        timer_exit(th);
        PutStr("done\n");
    } else {
        PutStr("no timer\n");
    }
    return 0;
}
