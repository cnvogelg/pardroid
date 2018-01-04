#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"

#include "uart.h"
#include "uartutil.h"
#include "rominfo.h"
#include "system.h"

#include "timer.h"
#include "spi.h"
#include "sdcard.h"
#include "ufat.h"
#include "crc.h"

static u08 sdbuf[512];
static ufat_disk_t disk;
static ufat_dir_entry_t entry;
static ufat_read_file_t read_file;

u08 ufat_io_read_block(u32 lba, u08 *data)
{
  u08 res = sdcard_read(lba, data);
  if(res == SDCARD_RESULT_OK) {
    return UFAT_RESULT_OK;
  } else {
    return UFAT_RESULT_READ_ERROR;
  }
}

u08 ufat_io_write_block(u32 lba, const u08 *data)
{
  u08 res = sdcard_write(lba, data);
  if(res == SDCARD_RESULT_OK) {
    return UFAT_RESULT_OK;
  } else {
    return UFAT_RESULT_WRITE_ERROR;
  }
}

static u08 dump_func(const ufat_dir_entry_t *e, void *user_data)
{
  uart_send_pstring(PSTR("entry:"));
  uart_send_hex_byte(e->type);
  uart_send('@');
  uart_send_hex_long(e->start_clus);
  uart_send('+');
  uart_send_hex_long(e->size_bytes);
  uart_send(':');
  uart_send_string((const char *)e->name);
  uart_send_crlf();
  return UFAT_SCAN_CONTINUE;
}

static void test_read_file(void)
{
  uart_send_pstring(PSTR("read_file_init:"));
  u08 res = ufat_read_file_init(&disk, &entry, &read_file);
  uart_send_hex_byte(res);
  uart_send_crlf();
  if(res != UFAT_RESULT_OK) {
    return;
  }

  uart_send_pstring(PSTR("read_file_next:"));
  u16 crc = 0;
  u32 total = 0;
  u08 *buf = disk.tmp_buf;
  u08 is_last = 0;
  while(!is_last) {
    u16 size;
    res = ufat_read_file_next(&disk, &read_file, &size);
    if(res != UFAT_RESULT_OK) {
      uart_send_hex_byte(res);
      uart_send_crlf();
      return;
    }
    if(size & UFAT_READ_FILE_EOF) {
      is_last = 1;
    }
    size &= UFAT_READ_FILE_SIZE_MASK;
    for(u16 i=0;i<size;i++) {
      crc = crc_xmodem_update(crc, buf[i]);
      total++;
    }
    uart_send('.');
  }
  uart_send_pstring(PSTR("total:"));
  uart_send_hex_long(total);
  uart_send_pstring(PSTR(",crc:"));
  uart_send_hex_word(crc);
  uart_send_crlf();
}

static void test_ufat(void)
{
  spi_init();

  // init card
  uart_send_pstring(PSTR("sdcard: "));
  u08 res = sdcard_init();
  uart_send_pstring(PSTR(" -> "));
  uart_send_hex_byte(res);
  uart_send_crlf();
  if(res != SDCARD_RESULT_OK) {
    return;
  }

  // init disk
  disk.tmp_buf = sdbuf;
  uart_send_pstring(PSTR("ufat_disk_init: "));
  res = ufat_disk_init(&disk);
  uart_send_hex_byte(res);
  uart_send_crlf();
  if(res != UFAT_RESULT_OK) {
    return;
  }

  // scan root dir
  uart_send_pstring(PSTR("ufat_root_scan:"));
  uart_send_crlf();
  res = ufat_root_scan(&disk, &entry, dump_func, NULL);
  uart_send_hex_byte(res);
  uart_send_crlf();
  if(res != UFAT_RESULT_OK) {
    return;
  }

  // find entry
  u08 *name1 = (u08 *)"test.hdf";
  uart_send_pstring(PSTR("ufat_root_find:"));
  uart_send_string((const char *)name1);
  res = ufat_root_find(&disk, &entry, name1);
  uart_send_hex_byte(res);
  uart_send_crlf();
  if(res == UFAT_RESULT_OK) {
    dump_func(&entry, NULL);
  }

  // find entry
  u08 *name = (u08 *)"readme.txt";
  uart_send_pstring(PSTR("ufat_root_find:"));
  uart_send_string((const char *)name);
  res = ufat_root_find(&disk, &entry, name);
  uart_send_hex_byte(res);
  uart_send_crlf();
  if(res == UFAT_RESULT_OK) {
    dump_func(&entry, NULL);
    test_read_file();
  }

  // find entry
  u08 *name2 = (u08 *)"wb311.hdf";
  uart_send_pstring(PSTR("ufat_root_find:"));
  uart_send_string((const char *)name2);
  res = ufat_root_find(&disk, &entry, name2);
  uart_send_hex_byte(res);
  uart_send_crlf();
  if(res == UFAT_RESULT_OK) {
    dump_func(&entry, NULL);
    test_read_file();
  }
}

int main(void)
{
  system_init();

  uart_init();
  uart_send_pstring(PSTR("parbox: test-ufat!"));
  uart_send_crlf();

  rom_info();

  test_ufat();

  for(int i=0;i<100;i++) {
    system_wdt_reset();
    uart_send('.');
    timer_delay(200);
  }

  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  system_sys_reset();
  return 0;
}
