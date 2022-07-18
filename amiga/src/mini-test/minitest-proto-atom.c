#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"
#include "proto_env.h"
#include "proto_atom.h"
#include "test/proto_atom.h"

int dosmain(void)
{
    proto_env_handle_t *penv;
    int error;

    PutStr("test-proto-atom\n");
    penv = proto_env_init((struct Library *)SysBase, &error);
    if(penv != NULL) {
        PutStr("proto env OK\n");

        /* setup proto low */
        PutStr("proto_atom_init\n");
        proto_handle_t *ph = proto_atom_init(penv);
        if(ph != NULL) {
            Printf("proto %ld\n", (ULONG)ph);

            // actions
            PutStr("action 1");
            error = proto_atom_action(ph, TEST_ACTION);
            Printf("-> %ld\n", (LONG)error);

            PutStr("action 2 (no busy)");
            error = proto_atom_action_no_busy(ph, TEST_ACTION);
            Printf("-> %ld\n", (LONG)error);

            PutStr("action 3 (bench)");
            ULONG delay[2];
            error = proto_atom_action_bench(ph, TEST_ACTION, delay);
            Printf("-> %ld t1=%ld  t2=%ld\n", (LONG)error, delay[0], delay[1]);

            // read/write word
            PutStr("read_word");
            UWORD wdata = 0;
            error = proto_atom_read_word(ph, TEST_READ_WORD, &wdata);
            Printf("-> %ld : %lx\n", (LONG)error, (ULONG)wdata);

            PutStr("write_word");
            error = proto_atom_write_word(ph, TEST_WRITE_WORD, 0xdead);
            Printf("-> %ld\n", (LONG)error);

            // read/write long
            PutStr("read_long");
            ULONG ldata = 0;
            error = proto_atom_read_long(ph, TEST_READ_LONG, &ldata);
            Printf("-> %ld : %lx\n", (LONG)error, ldata);

            PutStr("write_long");
            error = proto_atom_write_long(ph, TEST_WRITE_LONG, 0xcafebabe);
            Printf("-> %ld\n", (LONG)error);

            // read/write block
            PutStr("read_block");
            UBYTE buf[TEST_BUF_SIZE];
            error = proto_atom_read_block(ph, TEST_READ_BLOCK, buf, TEST_BUF_SIZE);
            Printf("-> %ld\n", (LONG)error);

            PutStr("write_block");
            error = proto_atom_write_block(ph, TEST_WRITE_BLOCK, buf, TEST_BUF_SIZE);
            Printf("-> %ld\n", (LONG)error);

            proto_atom_exit(ph);
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
