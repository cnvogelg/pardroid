#ifndef PROTO_LOW_H
#define PROTO_LOW_H

extern ASM int proto_low_ping(REG(a0, struct pario_port *port),
                              REG(a1, volatile UBYTE *timeout_flag),
                              REG(d0, UBYTE cmd));
extern ASM int proto_low_reg_read(REG(a0, struct pario_port *port),
                                  REG(a1, volatile UBYTE *timeout_flag),
                                  REG(d0, UBYTE cmd),
                                  REG(a2, UBYTE *data));
extern ASM int proto_low_reg_write(REG(a0, struct pario_port *port),
                                   REG(a1, volatile UBYTE *timeout_flag),
                                   REG(d0, UBYTE cmd),
                                   REG(a2, UBYTE *data));

#endif
