#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>

#include "bench.h"

bench_def_t *find_bench(bench_def_t *first, const char *name)
{
  bench_def_t *def = first;
  while(def->func != NULL) {
    if(strcmp(name, def->name) == 0) {
      return def;
    }
    def++;
  }
  Printf("Benchmark not found: %s\n", (LONG)name);
  return NULL;
}

void list_benches(bench_def_t *first)
{
  PutStr("--- Benchmarks ---\n");
  bench_def_t *def = first;
  while(def->func != NULL) {
    Printf("%-6s  %s\n", (LONG)def->name, (LONG)def->description);
    def++;
  }
}

static void write_header(bench_def_t *def)
{
  Printf("[.....]  (%-5s)  %s\r", (LONG)def->name, (LONG)def->description);
}

static void write_result(ULONG val)
{
  Printf("[%5ld]\n", val);
}

int bench_main(bench_def_t benches[], const char *name, void *user_data)
{
  // run single named test
  if(name != NULL) {
    bench_def_t *def = find_bench(benches, name);
    if(def == NULL) {
      list_benches(benches);
      return RETURN_WARN;
    }

    write_header(def);
    ULONG val = def->func(def, user_data);
    write_result(val);
  }
  // run all
  else {
    bench_def_t *def = benches;
    while(def->func != NULL) {
      write_header(def);
      ULONG val = def->func(def, user_data);
      write_result(val);
      def++;
    }
  }

  return RETURN_OK;
}

void bench_time_start(timer_handle_t *timer, stopwatch_t *watch)
{
  watch->end = 0;
  watch->delta = 0;
  watch->us = 0;
  watch->kbps = 0;
  timer_eclock_get(timer, &watch->start);
}

ULONG bench_time_stop(timer_handle_t *timer, stopwatch_t *watch, ULONG data_size)
{
  /* stop timer */
  timer_eclock_get(timer, &watch->end);
  timer_eclock_delta(&watch->end, &watch->start, &watch->delta);

  /* calc kbps */
  watch->us = timer_eclock_to_us(timer, &watch->delta);
  if(watch->us > 0) {
    watch->kbps = (data_size * 1000UL) / watch->us;
  }
  return watch->kbps;
}


