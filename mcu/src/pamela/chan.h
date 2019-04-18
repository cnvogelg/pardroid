#ifndef CHAN_H
#define CHAN_H

extern u16  chan_getclr_rx_pending(void);
extern u16  chan_getclr_error(void);

extern void chan_set_rx_pending(u16 mask);
extern void chan_set_error(u16 mask);

#endif
