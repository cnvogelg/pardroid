#ifndef PROTO_LOW_H
#define PROTO_LOW_H

extern ASM UBYTE proto_low_get_status(REG(a0, struct pario_port *port));

extern ASM int proto_low_knok_check(REG(a0, struct pario_port *port));

extern ASM int proto_low_knok_exit(REG(a0, struct pario_port *port),
                                   REG(a1, volatile UBYTE *timeout_flag));

extern ASM int proto_low_action(REG(a0, struct pario_port *port),
                                REG(a1, volatile UBYTE *timeout_flag),
                                REG(d0, UBYTE cmd));

/* benchmark data  */
struct cb_data;
typedef ASM void (*bench_cb_t)(REG(d0, int id), REG(a2, struct cb_data *cb));
typedef unsigned long long cb_ts_t;
struct cb_data {
  bench_cb_t    callback;
  void         *user_data;
  cb_ts_t       timestamps[3];
};

extern ASM int proto_low_action_bench(REG(a0, struct pario_port *port),
                                REG(a1, volatile UBYTE *timeout_flag),
                                REG(a2, struct cb_data *cbd),
                                REG(d0, UBYTE cmd));

extern ASM int proto_low_read_word(REG(a0, struct pario_port *port),
                                   REG(a1, volatile UBYTE *timeout_flag),
                                   REG(d0, UBYTE cmd),
                                   REG(a2, UWORD *data));
extern ASM int proto_low_write_word(REG(a0, struct pario_port *port),
                                    REG(a1, volatile UBYTE *timeout_flag),
                                    REG(d0, UBYTE cmd),
                                    REG(a2, UWORD *data));

extern ASM int proto_low_read_long(REG(a0, struct pario_port *port),
                                   REG(a1, volatile UBYTE *timeout_flag),
                                   REG(d0, UBYTE cmd),
                                   REG(a2, ULONG *data));
extern ASM int proto_low_write_long(REG(a0, struct pario_port *port),
                                    REG(a1, volatile UBYTE *timeout_flag),
                                    REG(d0, UBYTE cmd),
                                    REG(a2, ULONG *data));

extern ASM int proto_low_read_block(REG(a0, struct pario_port *port),
                                    REG(a1, volatile UBYTE *timeout_flag),
                                    REG(d0, UBYTE cmd),
                                    REG(a2, ULONG *msgiov));
extern ASM int proto_low_write_block(REG(a0, struct pario_port *port),
                                     REG(a1, volatile UBYTE *timeout_flag),
                                     REG(d0, UBYTE cmd),
                                     REG(a2, ULONG *msgiov));

#endif
