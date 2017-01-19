struct timer_handle;

extern struct timer_handle *timer_init(struct Library *SysBase);
extern void timer_exit(struct timer_handle *th);

extern volatile UBYTE *timer_get_flag(struct timer_handle *th);

extern void timer_start(struct timer_handle *th, ULONG secs, ULONG micros);
extern void timer_stop(struct timer_handle *th);

extern ULONG timer_get_eclock(struct timer_handle *th);
