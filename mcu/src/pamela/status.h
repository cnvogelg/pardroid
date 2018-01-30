#ifndef STATUS_H
#define STATUS_H

extern void status_init(void);
extern void status_handle(void);

// events
extern void status_clear_events(void);
extern u08  status_get_event_mask(void);
extern void status_set_event(u08 chn);

// attach/detach
extern void status_attach(void);
extern void status_detach(void);

// pending
extern void status_set_pending(u08 chn);
extern void status_clear_pending(u08 chn);

// busy
extern void status_set_busy(void);
extern void status_clear_busy(void);

#endif
