#ifndef TESTS_PROTO_H
#define TESTS_PROTO_H

#include "test.h"

void tests_proto_config(UWORD size, UWORD bias, UWORD add_size, UWORD sub_size,
                        UBYTE channel);

int test_reset(test_t *t, test_param_t *p);
int test_knok(test_t *t, test_param_t *p);
int test_ping(test_t *t, test_param_t *p);
int test_ping_busy(test_t *t, test_param_t *p);

int test_wfunc_write_read(test_t *t, test_param_t *p);
int test_lfunc_write_read(test_t *t, test_param_t *p);
int test_wfunc_busy(test_t *t, test_param_t *p);
int test_lfunc_busy(test_t *t, test_param_t *p);

int test_msg_empty(test_t *t, test_param_t *p);
int test_msg_tiny(test_t *t, test_param_t *p);
int test_msg_size(test_t *t, test_param_t *p);
int test_msg_size_max(test_t *t, test_param_t *p);
int test_msg_size_chunks(test_t *t, test_param_t *p);
int test_msg_write(test_t *t, test_param_t *p);
int test_msg_read(test_t *t, test_param_t *p);
int test_msg_write_too_large(test_t *t, test_param_t *p);
int test_msg_read_too_large(test_t *t, test_param_t *p);
int test_msg_write_busy(test_t *t, test_param_t *p);
int test_msg_read_busy(test_t *t, test_param_t *p);

int test_offset_write_read(test_t *t, test_param_t *p);
int test_offset_busy(test_t *t, test_param_t *p);

int test_timer_sig(test_t *t, test_param_t *p);
int test_event_sig(test_t *t, test_param_t *p);
int test_event_sig2(test_t *t, test_param_t *p);
int test_event_busy(test_t *t, test_param_t *p);
int test_event_rx_pending(test_t *t, test_param_t *p);
int test_event_error(test_t *t, test_param_t *p);
int test_event_msg(test_t *t, test_param_t *p);


#define TESTS_PAMELA_ALL \
  { test_reset, "reset", "reset parbox device" }, \
  { test_knok, "knok", "enter/leave knok mode of device" }, \
  { test_ping, "ping", "ping parbox device" }, \
  { test_ping_busy, "pingb", "ping while busy" }, \
  { test_wfunc_write_read, "wf", "write/read test function word" }, \
  { test_lfunc_write_read, "lf", "write/read test function long" }, \
  { test_wfunc_busy, "wfb", "write/read function word while busy" }, \
  { test_lfunc_busy, "lfb", "write/read function long while busy" }, \
  { test_msg_empty, "me", "write/read empty message"}, \
  { test_msg_tiny, "mt", "write/read tiny 4 byte message"}, \
  { test_msg_size, "ms", "write/read messages of given size"}, \
  { test_msg_size_max, "msx", "write/read messages of max size"}, \
  { test_msg_size_chunks, "msc", "write/read messages of given size in two chunks"}, \
  { test_msg_write, "mw", "write message of given size"}, \
  { test_msg_read, "mr", "read message of given size"}, \
  { test_msg_write_too_large, "mwtl", "write too large message"}, \
  { test_msg_read_too_large, "mrtl", "read too large message"}, \
  { test_msg_write_busy, "mwb", "write message while being busy"}, \
  { test_msg_read_busy, "mrb", "read message while being busy"}, \
  { test_offset_write_read, "o", "offset write/read" }, \
  { test_offset_busy, "ob", "offset write/read while busy" }, \
  { test_timer_sig, "tis", "timer signal"}, \
  { test_event_sig, "trs", "trigger signal"}, \
  { test_event_sig2, "trs2", "two trigger signals"}, \
  { test_event_busy, "tbs", "trigger after busy signal"}, \
  { test_event_rx_pending, "trp", "trigger after rx pending"}, \
  { test_event_error, "ter", "trigger after errror"}, \
  { test_event_msg, "tm", "trigger after message send"}, \

#endif
