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

struct proto_msg {
    UBYTE *data;         // +0: 4 data ptr
    UWORD  num_words;    // +4: 2 size
    UWORD  max_words;    // +6: 2 max size
};
typedef struct proto_msg proto_msg_t;

extern ASM int proto_low_msg_read(REG(a0, struct pario_port *port),
                                  REG(a1, volatile UBYTE *timeout_flag),
                                  REG(d0, UBYTE cmd),
                                  REG(a2, struct proto_msg *msg));
extern ASM int proto_low_msg_write(REG(a0, struct pario_port *port),
                                   REG(a1, volatile UBYTE *timeout_flag),
                                   REG(d0, UBYTE cmd),
                                   REG(a2, struct proto_msg *msg));

#endif
