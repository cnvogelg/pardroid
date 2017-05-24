#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "debug.h"

#include "engine.h"
#include "parbox.h"
#include "request.h"

static void test_write(engine_handle_t *eh, struct MsgPort *mp)
{
  /* create a request */
  request_t *r = request_create(mp);
  if(r != NULL) {
    /* fill in request */
    r->cmd = PB_REQ_MESSAGE_WRITE;
    r->channel = 0;
    r->data = "hallo!";
    r->length = 6;

    PutStr("send write request\n");
    engine_send_request(eh, r);
    PutStr("waiting for reply...\n");
    WaitPort(mp);
    request_t *r_get = (request_t *)GetMsg(mp);
    if(r_get == r) {
      Printf("returned write: error=%ld\n", r->error);
    } else {
      Printf("invalid request returned=%ld\n", r_get);
    }
    request_delete(r);
  } else {
    PutStr("no request!!\n");
  }
}

int dosmain(void)
{
  engine_handle_t *eh;

  PutStr("test-engine\n");
  int result;
  eh = engine_start(&result, (struct Library *)SysBase);
  if(eh != NULL) {
    PutStr("started ok.\n");

    /* open channel */
    result = engine_open_channel(eh, 0);
    if(result == 0) {
      PutStr("channel open\n");

      /* allocate reply message port */
      struct MsgPort *myPort = CreateMsgPort();
      if(myPort != NULL) {

        test_write(eh, myPort);

      } else {
        PutStr("no port!!\n");
      }
      result = engine_close_channel(eh, 0);
      Printf("channel close. result=%ld\n", result);
    } else {
      Printf("can't open channel 0!! result=%ld\n", result);
    }

    PutStr("stopping...\n");
    engine_stop(eh);
    PutStr("done\n");
  } else {
    PutStr("start: failed!\n");
    Printf("result=%ld -> %s\n", result, parbox_perror(result));
  }
  return 0;
}
