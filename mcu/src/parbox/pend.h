#ifndef PEND_H
#define PEND_H

#define PEND_RET_OK         0
#define PEND_RET_INVALID    1
#define PEND_RET_TOO_MANY   2

extern void pend_init(void);
extern void pend_handle(void);

extern u08 pend_add_req(u08 channel);
extern u08 pend_rem_req(u08 channel);
extern u08 pend_clear_reqs(u08 channel);

/* read only(!) values */
extern u16 pend_mask;
extern u16 pend_total;

#endif
