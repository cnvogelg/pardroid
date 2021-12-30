#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"
#include "proto_boot.h"

int dosmain(void)
{
    proto_env_handle_t *penv;
    int error;

    PutStr("test-proto-boot\n");
    penv = proto_env_init((struct Library *)SysBase, &error);
    if(penv != NULL) {
        PutStr("proto env OK\n");

        /* setup proto low */
        PutStr("proto_boot_init\n");
        proto_handle_t *ph = proto_boot_init(penv);
        if(ph != NULL) {
            Printf("proto %ld\n", (ULONG)ph);

            UWORD crc = 0;
            int res = proto_boot_get_rom_crc(ph, &crc);
            Printf("crc: ret=%ld crc=%x\n", res, crc);

            proto_boot_leave(ph);
            proto_boot_exit(ph);
            PutStr("proto done\n");
        } else {
            PutStr("error setting up proto!\n");
        }
        proto_env_exit(penv);
        PutStr("penv done\n");
    } else {
        Printf("error setting up proto env!\n");
    }
    PutStr("done\n");
    return 0;
}
