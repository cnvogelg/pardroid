#ifndef TIMER_H
#define TIMER_H

struct timer_handle;

extern struct timer_handle *timer_init(struct Library *SysBase);
extern void timer_exit(struct timer_handle *th);

extern volatile UBYTE *timer_get_flag(struct timer_handle *th);

extern void timer_start(struct timer_handle *th, ULONG secs, ULONG micros);
extern void timer_stop(struct timer_handle *th);

typedef unsigned long long time_stamp_t;

extern ULONG timer_eclock_get(struct timer_handle *th, time_stamp_t *val);
extern void timer_eclock_delta(time_stamp_t *end, time_stamp_t *begin, time_stamp_t *delta);
extern ULONG timer_eclock_to_us(struct timer_handle *th, time_stamp_t *et);
extern ULONG timer_eclock_to_bps(struct timer_handle *th, time_stamp_t *delta, ULONG bytes);
extern void timer_eclock_split(time_stamp_t *ts, ULONG *hi, ULONG *lo);

/* signal based timer */
extern BYTE timer_sig_init(struct timer_handle *th);
extern ULONG timer_sig_get_mask(struct timer_handle *th);
extern void timer_sig_exit(struct timer_handle *th);
extern void timer_sig_start(struct timer_handle *th, ULONG secs, ULONG micros);
extern void timer_sig_stop(struct timer_handle *th);

#endif
