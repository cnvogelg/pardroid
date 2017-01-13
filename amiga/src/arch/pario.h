struct pario_handle;

extern struct pario_handle *pario_init(struct Library *SysBase);
extern void pario_exit(struct pario_handle *ph);

extern int pario_setup_ack_irq(struct pario_handle *ph, struct Task *sigTask, BYTE signal);
extern void pario_cleanup_ack_irq(struct pario_handle *ph);
