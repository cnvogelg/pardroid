#ifndef TESTS_PROTO_H
#define TESTS_PROTO_H

#include "test.h"

void tests_proto_config(UWORD size, UWORD bias, UWORD add_size, UWORD sub_size, UBYTE channel);

int test_reset(test_t *t, test_param_t *p);
int test_ping(test_t *t, test_param_t *p);
int test_func_write(test_t *t, test_param_t *p);
int test_func_read(test_t *t, test_param_t *p);
int test_func_write_read(test_t *t, test_param_t *p);
int test_offset_write_read(test_t *t, test_param_t *p);
int test_msg_empty(test_t *t, test_param_t *p);
int test_msg_tiny(test_t *t, test_param_t *p);
int test_msg_size(test_t *t, test_param_t *p);
int test_msg_size_max(test_t *t, test_param_t *p);
int test_msg_size_chunks(test_t *t, test_param_t *p);
int test_msg_write(test_t *t, test_param_t *p);
int test_msg_read(test_t *t, test_param_t *p);
int test_msg_write_too_large(test_t *t, test_param_t *p);
int test_msg_read_too_large(test_t *t, test_param_t *p);
int test_status_timer_sig(test_t *t, test_param_t *p);
int test_status_reset_event_sig(test_t *t, test_param_t *p);
int test_status_read_pending(test_t *t, test_param_t *p);
int test_status_read_pending_sig(test_t *t, test_param_t *p);
int test_status_read_pending_two(test_t *t, test_param_t *p);
int test_status_read_pending_refresh(test_t *t, test_param_t *p);
int test_status_read_pending_refresh_active(test_t *t, test_param_t *p);
int test_status_read_pending_refresh_inactive(test_t *t, test_param_t *p);
int test_status_events(test_t *t, test_param_t *p);
int test_status_events_sig(test_t *t, test_param_t *p);
int test_status_events_in_pending(test_t *t, test_param_t *p);
int test_status_attach_detach(test_t *t, test_param_t *p);
int test_status_attach_detach_sig(test_t *t, test_param_t *p);
int test_status_attach_reset_sig(test_t *t, test_param_t *p);
int test_base_regs(test_t *t, test_param_t *p);

#define TESTS_PAMELA_ALL \
  { test_reset, "reset", "reset parbox device" }, \
  { test_ping, "ping", "ping parbox device" }, \
  { test_func_read, "fr", "read test function word" }, \
  { test_func_write, "fw", "write test function word" }, \
  { test_func_write_read, "fwr", "write/read test function word" }, \
  { test_offset_write_read, "owr", "write/read long offset" }, \
  { test_msg_empty, "me", "write/read empty message"}, \
  { test_msg_tiny, "mt", "write/read tiny 4 byte message"}, \
  { test_msg_size, "ms", "write/read messages of given size"}, \
  { test_msg_size_max, "msx", "write/read messages of max size"}, \
  { test_msg_size_chunks, "msc", "write/read messages of given size in two chunks"}, \
  { test_msg_write, "mw", "write message of given size"}, \
  { test_msg_read, "mr", "read message of given size"}, \
  { test_msg_write_too_large, "mwtl", "write too large message"}, \
  { test_msg_read_too_large, "mrtl", "read too large message"}, \
  { test_status_timer_sig, "sts", "status timeout signal"}, \
  { test_status_reset_event_sig, "srs", "status event right after reset"}, \
  { test_status_read_pending, "sp", "status read pending flag"}, \
  { test_status_read_pending_sig, "spi", "signal on read pending flag"}, \
  { test_status_read_pending_two, "spt", "read pending two channels"}, \
  { test_status_read_pending_refresh, "spr", "read pending refresh same channel"}, \
  { test_status_read_pending_refresh_active, "spr", "read pending refresh on active channel"}, \
  { test_status_read_pending_refresh_inactive, "spr", "read pending refresh on inactive channel"}, \
  { test_status_events, "se", "status event mask" }, \
  { test_status_events_sig, "sei", "signal on status event mask" }, \
  { test_status_events_in_pending, "seip", "event arrives while pending" }, \
  { test_status_attach_detach, "sad", "attach/detach" }, \
  { test_status_attach_detach_sig, "sadi", "signal on attach/detach" }, \
  { test_status_attach_reset_sig, "sari", "signal on attach/reset" }, \
  { test_base_regs, "br", "base regs" }, \

#endif
