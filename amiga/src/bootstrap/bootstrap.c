#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pamela.h"
#include "proto.h"

int dosmain(void)
{
  pamela_handle_t *pb;

  PutStr("parbox bootstrap\n");

  /* setup pamela */
  int init_res;
  pb = pamela_init((struct Library *)SysBase, &init_res);
  if(init_res == PAMELA_OK) {

    /* reset firmware */
    proto_handle_t *proto = pamela_get_proto(pb);
    res = proto_reset(proto, 1);
    if(res == PROTO_RET_OK) {

      /* run test */
      PutStr("OK!\n");

    } else {
      PutStr(proto_perror(res));
      PutStr(" -> ABORT\n");
    }

    pamela_exit(pb);
  } else {
    PutStr(pamela_perror(res));
    PutStr(" -> ABORT\n");
  }

  return 0;
}
