#ifndef PROTO_ATOM_H
#define PROTO_ATOM_H

// error codes
#define PROTO_RET_OK                0
#define PROTO_RET_RAK_INVALID       1
#define PROTO_RET_TIMEOUT           2
#define PROTO_RET_DEVICE_BUSY       3
#define PROTO_RET_ODD_BLOCK_SIZE    4

// handle
struct proto_handle;
typedef struct proto_handle proto_handle_t;

// init/exit of handle
extern proto_handle_t *proto_atom_init(struct pario_handle *ph, struct timer_handle *th, struct Library *SysBase);
extern void proto_atom_exit(proto_handle_t *ph);

// actions
extern int proto_atom_action(proto_handle_t *ph, UBYTE cmd);
extern int proto_atom_action_no_busy(proto_handle_t *ph, UBYTE cmd);
extern int proto_atom_action_bench(proto_handle_t *ph, UBYTE cmd, ULONG deltas[2]);

// read/write word
extern int proto_atom_read_word(proto_handle_t *ph, UBYTE cmd, UWORD *data);
extern int proto_atom_write_word(proto_handle_t *ph, UBYTE cmd, UWORD data);

// read/write long
extern int proto_atom_read_long(proto_handle_t *ph, UBYTE cmd, ULONG *data);
extern int proto_atom_write_long(proto_handle_t *ph, UBYTE cmd, ULONG data);

// read/write block
extern int proto_atom_read_block(proto_handle_t *ph, UBYTE chn, UBYTE *buf, UWORD num_bytes);
extern int proto_atom_write_block(proto_handle_t *ph, UBYTE chn, UBYTE *buf, UWORD num_bytes);

// verbose error
extern const char *proto_perror(int res);

#endif