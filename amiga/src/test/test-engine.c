#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include "autoconf.h"
#include "debug.h"

#include "engine.h"
#include "parbox.h"

int dosmain(void)
{
  engine_handle_t *eh;

  PutStr("test-engine\n");
  int result;
  eh = engine_start(&result, (struct Library *)SysBase);
  if(eh != NULL) {
    PutStr("started ok.\n");



    PutStr("stopping...\n");
    engine_stop(eh);
    PutStr("done\n");
  } else {
    PutStr("start: failed!\n");
    Printf("result=%ld -> %s\n", result, parbox_perror(result));
  }
  return 0;
}
