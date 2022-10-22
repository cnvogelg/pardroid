#include "autoconf.h"
#include "types.h"
#include "arch.h"

#define DEBUG 1

#include "debug.h"
#include "fwid.h"
#include "fw_info.h"

#include "hw_uart.h"
#include "hw_system.h"
#include "hw_led.h"
#include "hw_timer.h"

#include "uartutil.h"
#include "rominfo.h"

#include "sdcard.h"
#include "ff.h"

FW_INFO(FWID_TEST_FATFS, VERSION_TAG)

static FATFS fatfs;
static FIL file;
static DIR dir;

#define BUF_SIZE 2048

static u08 buf[BUF_SIZE];

static void test_fatfs(void)
{
  // acquire sd card
  uart_send_pstring(PSTR("sd card..."));
  u08 sd_res = sdcard_acquire(CONFIG_DRIVER_SDCARD_SPI_CS);
  uart_send_hex_byte(sd_res);
  uart_send_crlf();
  if(sd_res != SDCARD_RESULT_OK) {
    return;
  }

  // mount file system
  uart_send_pstring(PSTR("mounting..."));
  FRESULT res = f_mount(&fatfs, "", 0);
  uart_send_hex_word(res);
  uart_send_crlf();
  if(res == FR_OK) {

    // get free clusters
    uart_send_pstring(PSTR("free:"));
    DWORD free_clus;
    FATFS *ret_fatfs;
    res = f_getfree("", &free_clus, &ret_fatfs);
    uart_send_hex_word(res);
    hw_uart_send(':');
    uart_send_hex_long(free_clus);
    uart_send_crlf();

    // read dir
    uart_send_pstring(PSTR("opendir:"));
    res = f_opendir(&dir, "");
    uart_send_hex_word(res);
    uart_send_crlf();

    FILINFO fno;
    for (;;) {
      res = f_readdir(&dir, &fno);                   /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
      if (fno.fattrib & AM_DIR) {                    /* It is a directory */
        uart_send_pstring(PSTR("DIR "));
      } else {
        uart_send_hex_long(fno.fsize);
        hw_uart_send(' ');
      }
      uart_send_string(fno.fname);
      uart_send_crlf();
    }

    uart_send_pstring(PSTR("closedir:"));
    res = f_closedir(&dir);
    uart_send_hex_word(res);
    uart_send_crlf();

#define TEST_FILE "os134-wb.adf"

    // read file
    uart_send_pstring(PSTR("read file: " TEST_FILE));
    res = f_open(&file, TEST_FILE, FA_READ);
    uart_send_hex_word(res);
    uart_send_crlf();

    uart_send_pstring(PSTR("read"));
    UINT out;
    res = f_read(&file, buf, BUF_SIZE, &out);
    uart_send_hex_word(res);
    hw_uart_send('+');
    uart_send_hex_word(out);
    uart_send_crlf();

    uart_send_hex_dump(0, buf, BUF_SIZE);

    uart_send_pstring(PSTR("close"));
    res = f_close(&file);
    uart_send_hex_word(res);
    uart_send_crlf();

    // finally unmount file system
    uart_send_pstring(PSTR("unmounting..."));
    res = f_unmount("");
    uart_send_hex_word(res);
    uart_send_crlf();
  }

  // release sd card
  sdcard_release();
}

int main(void)
{
  hw_system_init();

  hw_uart_init();
  uart_send_pstring(PSTR("parbox: test-fatfs!"));
  uart_send_crlf();

  rom_info();

  test_fatfs();

  for(int i=0;i<100;i++) {
    hw_uart_send('.');
    hw_timer_delay_ms(200);
  }

  uart_send_pstring(PSTR("reset..."));
  uart_send_crlf();
  hw_system_reset();
  return 0;
}
