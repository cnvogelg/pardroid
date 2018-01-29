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
    Printf("%-6s  %s\n", def->name, def->description);
    def++;
  }
}

static void write_header(bench_def_t *def)
{
  Printf("[.....]  (%-5s)  %s\r", def->name, def->description);
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


