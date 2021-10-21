#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"
#include "pario.h"
#include "timer.h"
#include "proto_atom.h"

int dosmain(void)
{
    struct pario_handle *pario;
    struct timer_handle *timer;
    int error;

    PutStr("test-proto-atom\n");
    pario = pario_init((struct Library *)SysBase);
    if(pario != NULL) {
        PutStr("timer_init\n");
        timer = timer_init((struct Library *)SysBase);
        if(timer != NULL) {
            /* setup proto low */
            PutStr("proto_atom_init\n");
            proto_handle_t *ph = proto_atom_init(pario, timer, (struct Library *)SysBase);
            if(ph != NULL) {
                Printf("proto %ld\n", ph);

                // actions
                PutStr("action 1");
                error = proto_atom_action(ph, 1);
                Printf("-> %ld\n", (LONG)error);

                PutStr("action 2 (no busy)");
                error = proto_atom_action_no_busy(ph, 2);
                Printf("-> %ld\n", (LONG)error);

                PutStr("action 3 (bench)");
                ULONG delay[2];
                error = proto_atom_action_bench(ph, 3, delay);
                Printf("-> %ld t1=%ld  t2=%ld\n", (LONG)error, delay[0], delay[1]);

                // read/write word
                PutStr("read_word");
                UWORD wdata = 0;
                error = proto_atom_read_word(ph, 0x10, &wdata);
                Printf("-> %ld : %lx\n", (LONG)error, (ULONG)wdata);

                PutStr("write_word");
                error = proto_atom_write_word(ph, 0x11, 0xdead);
                Printf("-> %ld\n", (LONG)error);

                // read/write long
                PutStr("read_long");
                ULONG ldata = 0;
                error = proto_atom_read_long(ph, 0x20, &ldata);
                Printf("-> %ld : %lx\n", (LONG)error, ldata);

                PutStr("write_long");
                error = proto_atom_write_long(ph, 0x21, 0xcafebabe);
                Printf("-> %ld\n", (LONG)error);

                // read/write block
                PutStr("reaw_block");
                UBYTE buf[512];
                error = proto_atom_read_block(ph, 0x30, buf, 512);
                Printf("-> %ld\n", (LONG)error);

                PutStr("write_block");
                error = proto_atom_write_block(ph, 0x31, buf, 512);
                Printf("-> %ld\n", (LONG)error);

                proto_atom_exit(ph);
                PutStr("proto done\n");
            } else {
                PutStr("error setting up proto!\n");
            }
            timer_exit(timer);
            PutStr("timer done\n");
        } else {
            PutStr("error setting up timer!\n");
        }
        pario_exit(pario);
        PutStr("pario done\n");
    } else {
        PutStr("error setting up pario!\n");
    }
    PutStr("done\n");
    return 0;
}
