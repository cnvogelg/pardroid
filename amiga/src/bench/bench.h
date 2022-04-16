#ifndef BENCH_H
#define BENCH_H

#include "timer.h"

struct bench_def;

typedef ULONG (*bench_func_t)(struct bench_def *b, void *user_data);

typedef struct bench_def {
  bench_func_t    func;
  const char     *name;
  const char     *description;
} bench_def_t;

extern int bench_main(bench_def_t benches[], const char *name, void *user_data);

typedef struct stopwatch {
  time_stamp_t   start;
  time_stamp_t   end;
  time_stamp_t   delta;
  ULONG          us;
  ULONG          kbps;
} stopwatch_t;

extern void bench_time_start(timer_handle_t *timer, stopwatch_t *watch);
extern ULONG bench_time_stop(timer_handle_t *timer, stopwatch_t *watch, ULONG data_size);

#endif
