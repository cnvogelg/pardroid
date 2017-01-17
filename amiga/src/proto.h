struct proto_handle;

extern struct proto_handle *proto_init(struct pario_port *port, struct timer_handle *th);
extern void proto_exit(struct proto_handle *ph);

extern int proto_ping(struct proto_handle *ph);
