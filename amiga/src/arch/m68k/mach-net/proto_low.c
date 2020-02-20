#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pario.h"
#include "proto_iov.h"
#include "proto_low.h"

void proto_low_config_port(struct pario_port *port)
{

}

ASM int proto_low_action(REG(a0, struct pario_port *port),
                         REG(a1, volatile UBYTE *timeout_flag),
                         REG(d0, UBYTE cmd))
{
    return 0;
}

ASM int proto_low_action_no_busy(REG(a0, struct pario_port *port),
                                 REG(a1, volatile UBYTE *timeout_flag),
                                 REG(d0, UBYTE cmd))
{
    return 0;
}

ASM int proto_low_action_bench(REG(a0, struct pario_port *port),
                                REG(a1, volatile UBYTE *timeout_flag),
                                REG(a2, struct cb_data *cbd),
                                REG(d0, UBYTE cmd))
{
    return 0;
}

ASM int proto_low_read_word(REG(a0, struct pario_port *port),
                                   REG(a1, volatile UBYTE *timeout_flag),
                                   REG(d0, UBYTE cmd),
                                   REG(a2, UWORD *data))
{
    return 0;
}

ASM int proto_low_write_word(REG(a0, struct pario_port *port),
                                    REG(a1, volatile UBYTE *timeout_flag),
                                    REG(d0, UBYTE cmd),
                                    REG(a2, UWORD *data))
{
    return 0;
}

ASM int proto_low_read_long(REG(a0, struct pario_port *port),
                                   REG(a1, volatile UBYTE *timeout_flag),
                                   REG(d0, UBYTE cmd),
                                   REG(a2, ULONG *data))
{
    return 0;
}

ASM int proto_low_write_long(REG(a0, struct pario_port *port),
                                    REG(a1, volatile UBYTE *timeout_flag),
                                    REG(d0, UBYTE cmd),
                                    REG(a2, ULONG *data))
{
    return 0;
}

ASM int proto_low_read_block(REG(a0, struct pario_port *port),
                                    REG(a1, volatile UBYTE *timeout_flag),
                                    REG(d0, UBYTE cmd),
                                    REG(a2, proto_iov_t *msgiov),
                                    REG(d1, UWORD num_words))
{
    return 0;
}

ASM int proto_low_write_block(REG(a0, struct pario_port *port),
                                     REG(a1, volatile UBYTE *timeout_flag),
                                     REG(d0, UBYTE cmd),
                                     REG(a2, proto_iov_t *msgiov),
                                     REG(d1, UWORD num_words))
{
    return 0;
}
