#ifndef PROTO_ATOM_TESTSUITE_ATOM_H
#define PROTO_ATOM_TESTSUITE_ATOM_H

#include "test.h"

int test_action(test_t *t, test_param_t *p);
int test_action_no_busy(test_t *t, test_param_t *p);
int test_action_bench(test_t *t, test_param_t *p);
int test_read_word(test_t *t, test_param_t *p);
int test_write_word(test_t *t, test_param_t *p);
int test_read_long(test_t *t, test_param_t *p);
int test_write_long(test_t *t, test_param_t *p);
int test_read_block(test_t *t, test_param_t *p);
int test_write_block(test_t *t, test_param_t *p);

int test_busy_action(test_t *t, test_param_t *p);
int test_busy_word(test_t *t, test_param_t *p);
int test_busy_long(test_t *t, test_param_t *p);
int test_busy_block(test_t *t, test_param_t *p);

#define TESTS_PROTO_ATOM \
  { test_action, "a", "action"}, \
  { test_action_no_busy, "anb", "action no busy"}, \
  { test_action_bench, "ab", "action bench"}, \
  { test_read_word, "rw", "read word"}, \
  { test_write_word, "ww", "write word"}, \
  { test_read_long, "rl", "read long"}, \
  { test_write_long, "wl", "write long"}, \
  { test_read_block, "rb", "read block"}, \
  { test_write_block, "wb", "write block"}, \
  { test_busy_action, "ba", "busy vs action"}, \
  { test_busy_word, "bw", "busy vs rw word"}, \
  { test_busy_word, "bl", "busy vs rw long"}, \
  { test_busy_word, "bb", "busy vs rw block"}, \

#endif
