#ifndef STATUS_H
#define STATUS_H

extern void status_init(void);
extern void status_update(void);
extern void status_handle(void);

extern u08  status_get_current(void);

extern void status_clear_events(void);
extern u08  status_get_event_mask(void);
extern void status_set_event(u08 chn);

extern void status_attach(void);
extern void status_detach(void);

// DEBUG
extern void status_set_pending(u08 pmask);
extern void status_clear_pending(void);
extern void status_reset_pending(void);

extern u08  status_get_pending_mask(void);
extern void status_set_pending_mask(u08 mask);
extern void status_clear_pending_mask(u08 mask);

#endif
