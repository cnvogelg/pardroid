#ifndef TIMER_H
#define TIMER_H

struct timer_handle;

extern struct timer_handle *timer_init(struct Library *SysBase);
extern void timer_exit(struct timer_handle *th);

extern volatile UBYTE *timer_get_flag(struct timer_handle *th);

extern void timer_start(struct timer_handle *th, ULONG secs, ULONG micros);
extern void timer_stop(struct timer_handle *th);

struct time_stamp {
    ULONG hi;
    ULONG lo;
};
typedef struct time_stamp time_stamp_t;

extern ULONG timer_get_eclock(struct timer_handle *th, time_stamp_t *val);
extern void timer_delta(struct timer_handle *th, time_stamp_t *end, time_stamp_t *begin);
extern ULONG timer_eclock_to_us(struct timer_handle *th, time_stamp_t *et);
extern ULONG timer_calc_bps(struct timer_handle *th, time_stamp_t *delta, ULONG bytes);

#endif
