struct pario_handle;

struct pario_handle *pario_init(struct Library *SysBase);
void pario_exit(struct pario_handle *ph);

int pario_setup_ack_irq(struct pario_handle *ph, struct Task *sigTask, BYTE signal);
void pario_cleanup_ack_irq(struct pario_handle *ph);
