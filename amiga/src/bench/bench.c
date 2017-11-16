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
  Printf("Benchmark not found: %s\n", name);
  return NULL;
}

void list_benches(bench_def_t *first)
{
  PutStr("--- Benchmarks ---\n");
  bench_def_t *def = first;
  while(def->func != NULL) {
    Printf("%20s  %s\n", def->name, def->description);
    def++;
  }
}

int bench_main(bench_def_t benches[], const char *name, void *user_data)
{
  bench_def_t *def = find_bench(benches, name);
  if(def == NULL) {
    list_benches(benches);
    return RETURN_WARN;
  }

  def->func(user_data);

  return RETURN_OK;
}
