#include "proto_shared.h"

#define TEST_PAMELA_ACTION_TRIGGER_SIGNAL       (PROTO_ACTION_USER+0)
#define TEST_PAMELA_ACTION_BUSY_LOOP            (PROTO_ACTION_USER+1)

#define TEST_PAMELA_WFUNC_READ_WRITE            (PROTO_WFUNC_USER+0)
#define TEST_PAMELA_WFUNC_MAX_BYTES             (PROTO_WFUNC_USER+1)
#define TEST_PAMELA_WFUNC_BUF_WORDS             (PROTO_WFUNC_USER+2)
#define TEST_PAMELA_WFUNC_CHAN_RX_PEND          (PROTO_WFUNC_USER+3)
#define TEST_PAMELA_WFUNC_CHAN_ERROR            (PROTO_WFUNC_USER+4)