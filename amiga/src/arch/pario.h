#ifndef PARIO_H
#define PARIO_H

struct pario_handle;
struct pario_port;

extern struct pario_handle *pario_init(struct Library *SysBase);
extern void pario_exit(struct pario_handle *ph);

extern int pario_setup_ack_irq(struct pario_handle *ph, struct Task *sigTask, BYTE signal);
extern void pario_cleanup_ack_irq(struct pario_handle *ph);

extern struct pario_port *pario_get_port(struct pario_handle *ph);

extern UWORD pario_get_ack_irq_counter(struct pario_handle *ph);
extern UWORD pario_get_signal_counter(struct pario_handle *ph);
extern void pario_confirm_ack_irq(struct pario_handle *ph);

#endif
