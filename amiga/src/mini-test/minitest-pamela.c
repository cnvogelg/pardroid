#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pamela.h"

#define PORT 1234
#define SIZE 6

static void handle_error(int error)
{
  if(error != PAMELA_OK) {
    Printf("pamela error: %ld %s\n", error, pamela_perror(error));
  }
}

int dosmain(void)
{
  pamela_handle_t *ph;
  pamela_channel_t *ch;
  int error = 0;
  char hello[] = "hello!";

  PutStr("test-pamela\n");
  ph = pamela_init((struct Library *)SysBase, &error);
  handle_error(error);
  if(ph != NULL) {
    PutStr("pamela OK\n");

    // try to open a channel
    ch = pamela_open(ph, PORT, &error);
    handle_error(error);
    if(ch != NULL) {

      // write something
      PutStr("write something\n");
      error = pamela_write_request(ch, SIZE);
      handle_error(error);
      if(error == PAMELA_OK) {
        PutStr("wait for event\n");
        int wait = pamela_event_wait(ph, 1, 0, NULL);
        if(wait == PAMELA_WAIT_EVENT) {
          PutStr("got event\n");
          error = pamela_event_update(ph);
          handle_error(error);
          if(error == PAMELA_OK) {
            PutStr("got update\n");
            error = pamela_write_data(ch, hello);
            handle_error(error);
            PutStr("wrote data\n");
          }
        }
      }

      // read something
      PutStr("read something\n");
      error = pamela_read_request(ch, SIZE);
      handle_error(error);
      if(error == PAMELA_OK) {
        PutStr("wait for event\n");
        int wait = pamela_event_wait(ph, 1, 0, NULL);
        if(wait == PAMELA_WAIT_EVENT) {
         PutStr("got event\n");
          error = pamela_event_update(ph);
          handle_error(error);
          if(error == PAMELA_OK) {
            PutStr("got update\n");
            error = pamela_read_data(ch, hello);
            handle_error(error);
            Printf("read data: %s\n", hello);
          }
        }
      }

      error = pamela_close(ch);
      handle_error(error);
    }

    pamela_exit(ph);
    PutStr("pamela done\n");
  }

  return 0;
}
