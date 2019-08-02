#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>

#include "test.h"

static void print_header(test_t *test, test_param_t *param)
{
  Printf("[....]  (%-5s)  %s\r", (LONG)test->name, (LONG)test->description);
}

static void print_error(int res, test_param_t *param)
{
  Printf("[FAIL]\n"
         "        error=%ld  %s  section=%s\n",
    (LONG)res, (LONG)param->error, (LONG)param->section);
  if(param->extra != NULL) {
    Printf("        %s\n", (LONG)param->extra);
  }
}

static void clear_error(test_param_t *param)
{
  param->error = NULL;
  param->section = NULL;
  param->extra[0] = 0;
}

static int test_run_single(test_t *test, test_param_t *param, int silent, int num)
{
  /* report test */
  if(!silent) {
    print_header(test, param);
  }

  int res;
  if(num == 1) {
    /* init test state */
    clear_error(param);
    param->iter = 0;

    /* single run */
    res = test->func(test, param);
    if(res == 0) {
      if(!silent) {
        PutStr("[ ok ]\n");
      }
    }
    else {
      if(silent) {
        print_header(test, param);
      }
      print_error(res, param);
    }
  }
  else {
    /* loop */
    ULONG cnt = 0;
    ULONG num_failed = 0;
    ULONG delay = param->delay;
    while(1) {
      /* break? */
      if(SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) {
        PutStr("\nAbort.\n");
        break;
      }

      /* init test state */
      clear_error(param);
      param->iter = cnt++;

      Printf("[%04ld]\r", cnt);

      /* test func */
      res = test->func(test, param);
      if(res != 0) {
        num_failed++;
      }

      /* done? */
      if(num == cnt) {
        break;
      }

      /* delay */
      if(delay > 0) {
        Delay(delay);
      }
    }
    PutStr("\n----- Loop Result -----\n");
    Printf("Total: %ld, Failed: %ld\n", cnt, num_failed);
  }
  return res == 0 ? RETURN_OK : RETURN_WARN;
}

static int test_run_all(test_t all_tests[], test_param_t *param, int silent)
{
  ULONG cnt = 0;
  ULONG num_failed = 0;
  int final_res = RETURN_OK;

  if(!silent) {
    PutStr("----- All Tests -----\n");
  }
  test_t *t = all_tests;
  while(t->name != NULL) {
    int res = test_run_single(t, param, silent, 1);
    if(res != RETURN_OK) {
      num_failed++;
      final_res = res;
    }
    t++;
    cnt++;
  }

  if(!silent || num_failed > 0) {
    if(silent) {
      PutStr("\n");
    }
    Printf("----- Result -----\n");
    Printf("Total: %ld, Failed: %ld\n", cnt, num_failed);
  }

  return final_res;
}

static test_t *pick_test(test_t all_tests[], const char *name)
{
  /* default */
  if(name == NULL) {
    return &all_tests[0];
  }
  /* search test */
  test_t *t = all_tests;
  while(t->name != NULL) {
    if(strcmp(name, t->name) == 0) {
      return t;
    }
    t++;
  }
  return NULL;
}

static void show_tests(test_t all_tests[])
{
  test_t *t = all_tests;
  while(t->name != NULL) {
    Printf("%-16s  %s\n", (LONG)t->name, (LONG)t->description);
    t++;
  }
}

int test_main(test_t all_tests[], test_param_t *param)
{
  /* run a single test? */
  if(param->test_name) {
    /* pick test function */
    test_t *test = pick_test(all_tests, param->test_name);
    if(test == NULL) {
      PutStr("Test not found! Available tests are:\n");
      show_tests(all_tests);
      return RETURN_WARN;
    } else {
      /* run single test */
      return test_run_single(test, param, 0, param->num_iter);
    }
  }
  /* run all tests */
  else {
    /* loop all tests */
    if(param->num_iter != 1) {
      /* loop */
      ULONG cnt = 0;
      ULONG num_failed = 0;
      int final_res = RETURN_OK;
      PutStr("----- Loop All Tests -----\n");
      while(1) {
        /* break? */
        if(SetSignal(0L,SIGBREAKF_CTRL_C) & SIGBREAKF_CTRL_C) {
          PutStr("Abort.\n");
          break;
        }

        Printf("#%04ld\r", cnt);
        int res = test_run_all(all_tests, param, !param->verbose);
        if(res != RETURN_OK) {
          final_res = res;
          num_failed++;
        }

        cnt++;
        if(cnt == param->num_iter) {
          break;
        }
      }
      PutStr("----- Result -----\n");
      Printf("Total: %ld, Failed: %ld\n", cnt, num_failed);
    }
    /* run all tests a single time */
    else {
      return test_run_all(all_tests, param, 0);
    }
  }
}

