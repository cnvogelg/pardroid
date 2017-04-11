#ifndef PROTO_LOW_H
#define PROTO_LOW_H

extern ASM int proto_low_action(REG(a0, struct pario_port *port),
                                REG(a1, volatile UBYTE *timeout_flag),
                                REG(d0, UBYTE cmd));

/* benchmark data  */
struct cb_data;
typedef ASM void (*bench_cb_t)(REG(d0, int id), REG(a2, struct cb_data *cb));
struct cb_ts {
  ULONG        hi;
  ULONG        lo;
};
struct cb_data {
  bench_cb_t    callback;
  void         *user_data;
  struct cb_ts  timestamps[3];
};

extern ASM int proto_low_action_bench(REG(a0, struct pario_port *port),
                                REG(a1, volatile UBYTE *timeout_flag),
                                REG(a2, struct cb_data *cbd),
                                REG(d0, UBYTE cmd));

extern ASM int proto_low_read_word(REG(a0, struct pario_port *port),
                                   REG(a1, volatile UBYTE *timeout_flag),
                                   REG(d0, UBYTE cmd),
                                   REG(a2, UBYTE *data));
extern ASM int proto_low_write_word(REG(a0, struct pario_port *port),
                                    REG(a1, volatile UBYTE *timeout_flag),
                                    REG(d0, UBYTE cmd),
                                    REG(a2, UBYTE *data));

extern ASM int proto_low_read_block(REG(a0, struct pario_port *port),
                                    REG(a1, volatile UBYTE *timeout_flag),
                                    REG(d0, UBYTE cmd),
                                    REG(a2, ULONG *msgiov));
extern ASM int proto_low_write_block(REG(a0, struct pario_port *port),
                                     REG(a1, volatile UBYTE *timeout_flag),
                                     REG(d0, UBYTE cmd),
                                     REG(a2, ULONG *msgiov));

#endif
