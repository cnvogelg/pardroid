#ifndef STATUS_H
#define STATUS_H

extern void status_set_busy(void);
extern void status_clr_busy(void);
extern u08 status_is_busy(void);

extern void status_set_rx_pending(u08 chan);
extern void status_clr_rx_pending(u08 chan);
extern void status_set_error(u08 chan);
extern void status_clr_error(u08 chan);

extern void status_set_rx_pending_mask(u16 mask);
extern void status_set_error_mask(u16 mask);

extern void status_set_mask(u32 mask);
extern u32 status_get_mask(void);

#endif
