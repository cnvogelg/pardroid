#ifndef STATUS_H
#define STATUS_H

extern void status_set_busy(void);
extern void status_clr_busy(void);
extern u08 status_is_busy(void);

extern void status_set_rx_flag(u08 chan);
extern void status_clr_rx_flag(u08 chan);
extern void status_set_error_flag(u08 chan);
extern void status_clr_error_flag(u08 chan);

/* debugging */
extern void status_set_status_mask(u16 mask);
extern void status_set_error_mask(u16 mask);

extern u16 status_get_status_mask(void);
extern u16 status_get_error_mask(void);

#endif
