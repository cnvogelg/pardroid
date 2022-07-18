#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "types.h"
#include "arch.h"

#include "timer.h"
#include "pario.h"
#include "proto_boot.h"
#include "proto-boot-testsuite.h"
#include "proto/wire_boot.h"
#include "test/proto_boot.h"
#include "test-buffer.h"

#define TEST_BYTE_OFFSET 2
#define TEST_PAGE_ADDR   0xbeefdeadUL

TEST_FUNC(test_page_rom_size)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;
  UWORD page_size = 0;
  ULONG rom_size = 0;

  // get page size
  int res = proto_boot_get_page_size(proto, &page_size);
  CHECK_RES(res, "get_page_size");
  CHECK_EQUAL(page_size, TEST_PAGE_SIZE, "page size");

  // get rom size
  res = proto_boot_get_rom_size(proto, &rom_size);
  CHECK_RES(res, "get_rom_size");
  CHECK_LEQUAL(rom_size, TEST_ROM_SIZE, "rom size");

  return 0;
}

TEST_FUNC(test_rom_param)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;
  UWORD val = 0;

  // get rom crc
  int res = proto_boot_get_rom_crc(proto, &val);
  CHECK_RES(res, "get_rom_crc");
  CHECK_EQUAL(val, TEST_ROM_CRC, "rom crc");

  // get rom mach tag
  res = proto_boot_get_rom_mach_tag(proto, &val);
  CHECK_RES(res, "get_rom_mach_tag");
  CHECK_EQUAL(val, TEST_ROM_MACH_TAG, "rom mach tag");

  // get rom fw version
  res = proto_boot_get_rom_fw_version(proto, &val);
  CHECK_RES(res, "get_rom_fw_version");
  CHECK_EQUAL(val, TEST_ROM_FW_VERSION, "rom fw version");

  // get rom fw id
  res = proto_boot_get_rom_fw_id(proto, &val);
  CHECK_RES(res, "get_rom_fw_id");
  CHECK_EQUAL(val, TEST_ROM_FW_ID, "rom fw id");

  return 0;
}

TEST_FUNC(test_write_read_page)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;

  UBYTE *buf = test_buffer_alloc(TEST_PAGE_SIZE, p);
  if (buf == NULL)
  {
    return 1;
  }
  for(int i=0;i<TEST_PAGE_SIZE;i++) {
    UBYTE val = (UBYTE)((i + TEST_BYTE_OFFSET) & 0xff);
    buf[i] = val;
  }

  // write buffer
  int res = proto_boot_write_page(proto, buf, TEST_PAGE_SIZE);
  CHECK_RES(res, "write_page");

  // read buffer
  res = proto_boot_read_page(proto, buf, TEST_PAGE_SIZE);
  CHECK_RES(res, "read_data");

  // check buffer
  int errors = 0;
  for(int i=0;i<TEST_PAGE_SIZE;i++) {
    UBYTE val = (UBYTE)((i + TEST_BYTE_OFFSET) & 0xff);
    if (buf[i] != val)
    {
      p->error = "value mismatch";
      p->section = "compare";
      sprintf(p->extra, "@%ld: w=%04lx r=%04lx (errors=%d)", (LONG)i, (ULONG)val, (ULONG)buf[i], errors);
      errors ++;
    }
  }

  test_buffer_free(buf);

  if(errors > 0) {
    return 1;
  }
  
  return 0;
}

TEST_FUNC(test_page_addr)
{
  proto_handle_t *proto = (proto_handle_t *)p->user_data;

  UBYTE *buf = test_buffer_alloc(TEST_PAGE_SIZE, p);
  if (buf == NULL)
  {
    return 1;
  }

  // set addr
  int res = proto_boot_set_page_addr(proto, TEST_PAGE_ADDR);
  CHECK_RES(res, "set_page_addr");

  // read buf with addr pasted in
  res = proto_boot_read_page(proto, buf, TEST_PAGE_SIZE);
  CHECK_RES(res, "read_page");

  // check for addr in buf
  ULONG *val = (ULONG *)buf;
  CHECK_LEQUAL(*val, TEST_PAGE_ADDR, "page addr");

  test_buffer_free(buf);

  return 0;
}
