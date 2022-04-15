#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pamela.h"

#define PORT 1010
#define SIZE 6

static void handle_error(int error)
{
  if(error > 0) {
    Printf("GOT: %ld bytes\n", error);
  }
  else if(error != PAMELA_OK) {
    Printf("pamela error: %ld %s\n", error, pamela_perror(error));
  } else {
    PutStr("OK\n");
  }
}

static int wait_event(pamela_handle_t *ph, pamela_channel_t *ch)
{
  PutStr("[event wait:");
  int wait = pamela_event_wait(ph, 10, 0, NULL);
  if(wait == PAMELA_WAIT_EVENT) {
    UWORD mask = 0;
    int error = pamela_event_update(ph, &mask);
    handle_error(error);
    if(error == PAMELA_OK) {
      Printf("mask=%08lx]\n", (ULONG)mask);
    } else {
      Printf("error: %ld]", error);
    }
    return error;
  } else {
    Printf("wrong result: %ld]\n", wait);
    return PAMELA_ERROR_UNKNOWN;
  }
}

int dosmain(void)
{
  pamela_handle_t *ph;
  pamela_channel_t *ch;
  int error = 0;
  char hello[] = "hello!";

  PutStr("pamela init\n");
  ph = pamela_init((struct Library *)SysBase, &error);
  handle_error(error);
  if(ph != NULL) {

    // try to open a channel
    PutStr("open\n");
    ch = pamela_open(ph, PORT, &error);
    handle_error(error);
    if(ch != NULL) {

      // async wait for open
      wait_event(ph, ch);

      // do a reset
      PutStr("reset\n");
      error = pamela_reset(ch);
      handle_error(error);

      // async wait for reset
      wait_event(ph, ch);

      // set mtu
      PutStr("set_mtu");
      error = pamela_set_mtu(ch, 88);
      handle_error(error);

      // get mtu
      PutStr("get_mtu");
      UWORD mtu = 0;
      error = pamela_get_mtu(ch, &mtu);
      handle_error(error);
      Printf("mtu=%ld\n", mtu);

      // seek
      PutStr("seek\n");
      error = pamela_seek(ch, 0xdeadbeef);
      handle_error(error);

      // tell
      PutStr("tell\n");
      ULONG pos = 0;
      error = pamela_tell(ch, &pos);
      handle_error(error);
      Printf("pos=%08lx\n", pos);

      // write something
      PutStr("write req\n");
      error = pamela_write_request(ch, SIZE);
      handle_error(error);

      error = wait_event(ph, ch);
      if(error == PAMELA_OK) {
        PutStr("write data\n");
        error = pamela_write_data(ch, hello);
        handle_error(error);
      }

      // read something
      PutStr("read req\n");
      error = pamela_read_request(ch, SIZE);
      handle_error(error);

      error = wait_event(ph, ch);
      if(error == PAMELA_OK) {
        PutStr("read data\n");
        error = pamela_read_data(ch, hello);
        handle_error(error);
      }

      PutStr("close\n");
      error = pamela_close(ch);
      handle_error(error);

      // async wait for close
      wait_event(ph, ch);
    }

    PutStr("pamela exit\n");
    pamela_exit(ph);
    PutStr("pamela done\n");
  }

  return 0;
}
