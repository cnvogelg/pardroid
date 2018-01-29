#ifndef BENCH_MAIN_H
#define BENCH_MAIN_H

extern void *bench_api_init(void);
extern void bench_api_exit(void *user_data);

extern ULONG bench_get_num(void);
extern ULONG bench_get_size(void);
extern ULONG bench_get_verbose(void);

extern bench_def_t all_benches[];

#endif
