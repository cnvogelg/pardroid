#ifndef STATUS_H
#define STATUS_H

extern void status_set_busy(void);
extern void status_clr_busy(void);
extern u08 status_is_busy(void);

extern void status_set_rx_pending(u16 mask);
extern void status_set_error(u16 mask);

extern void status_set_mask(u32 mask);
extern u32 status_get_mask(void);

#endif
