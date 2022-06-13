#ifndef PAMELA_ENGINE_TESTSUITE_H
#define PAMELA_ENDING_TESTSUITE_H

#include "test.h"

TEST_FUNC(test_init_exit);
TEST_FUNC(test_open_close);
TEST_FUNC(test_write);
TEST_FUNC(test_read);
TEST_FUNC(test_seek_tell);

#define TESTS_PAMELA \
  { test_init_exit, "ie", "init/exit pamela engine" }, \
  { test_open_close, "oc", "open/close channel" }, \
  { test_write, "w", "write message on channel" }, \
  { test_read, "r", "read message from channel" }, \
  { test_seek_tell, "st", "seek/tell" }, \
 
struct pam_eng_test_data {
  pamela_engine_t  *engine;
  struct MsgPort   *port;
  struct IOPamReq  *req;
};
typedef struct pam_eng_test_data pam_eng_test_data_t;

#endif
