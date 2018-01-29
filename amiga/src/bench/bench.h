#ifndef BENCH_H
#define BENCH_H

struct bench_def;

typedef ULONG (*bench_func_t)(struct bench_def *b, void *user_data);

typedef struct bench_def {
  bench_func_t    func;
  const char     *name;
  const char     *description;
} bench_def_t;

extern int bench_main(bench_def_t benches[], const char *name, void *user_data);

#endif
