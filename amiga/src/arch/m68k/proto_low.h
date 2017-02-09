#ifndef PROTO_LOW_H
#define PROTO_LOW_H

extern ASM int proto_low_ping(REG(a0, struct pario_port *port),
                              REG(a1, volatile UBYTE *timeout_flag));
extern ASM int proto_low_test_read(REG(a0, struct pario_port *port),
                                   REG(a1, volatile UBYTE *timeout_flag),
                                   REG(a2, UBYTE *data));
extern ASM int proto_low_test_write(REG(a0, struct pario_port *port),
                                    REG(a1, volatile UBYTE *timeout_flag),
                                    REG(a2, UBYTE *data));

#endif
