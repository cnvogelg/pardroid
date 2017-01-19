struct proto_handle;

#define CMD_IDLE 0

extern struct proto_handle *proto_init(struct pario_port *port, struct timer_handle *th);
extern void proto_exit(struct proto_handle *ph);

extern int proto_ping(struct proto_handle *ph);
extern int proto_test_read(struct proto_handle *ph, UBYTE *data);
extern int proto_test_write(struct proto_handle *ph, UBYTE *data);
