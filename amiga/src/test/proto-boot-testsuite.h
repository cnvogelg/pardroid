#ifndef PROTO_ATOM_TESTSUITE_DEV_H
#define PROTO_ATOM_TESTSUITE_DEV_H

#include "test.h"

TEST_FUNC(test_page_rom_size);
TEST_FUNC(test_rom_param);
TEST_FUNC(test_write_read_page);
TEST_FUNC(test_page_addr);

#define TESTS_PROTO_DEV \
  { test_page_rom_size, "prs", "page/rom size"}, \
  { test_rom_param, "rp", "rom parameter"}, \
  { test_write_read_page, "wrp", "write read page"}, \
  { test_page_addr, "pa", "page addr" }, \

#endif
