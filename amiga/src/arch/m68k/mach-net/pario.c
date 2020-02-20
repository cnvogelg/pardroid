#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pario.h"

struct pario_handle *pario_init(struct Library *SysBase)
{
    return NULL;
}

void pario_exit(struct pario_handle *ph)
{
}

int pario_setup_ack_irq(struct pario_handle *ph, struct Task *sigTask, BYTE signal)
{
    return 0;
}

void pario_cleanup_ack_irq(struct pario_handle *ph)
{
}

struct pario_port *pario_get_port(struct pario_handle *ph)
{
    return NULL;
}

UWORD pario_get_ack_irq_counter(struct pario_handle *ph)
{
    return 0;
}

UWORD pario_get_signal_counter(struct pario_handle *ph)
{
    return 0;
}

void pario_confirm_ack_irq(struct pario_handle *ph)
{
}
