#include <proto/exec.h>
#include <proto/dos.h>

#include "sim_msg.h"

#define MY_PORT 1234

int dosmain(void)
{
  struct sim_msg_handle sm_hnd = { NULL };
  struct sim_msg msg;

  PutStr("pario_sim_test\n");
  if(sim_msg_client_init(&sm_hnd)!=0) {
    PutStr("sim msg setup failed!\n");
    return 10;
  }

  /* send status */
  UBYTE status = 0;
  int res = sim_msg_client_do_status(&sm_hnd, &status);
  if(res == 0) {
    Printf("status=%ld\n", (LONG)status);
  } else {
    Printf("ERROR status: %ld\n", res);
  }

  PutStr("exit\n");
  sim_msg_client_exit(&sm_hnd);
  PutStr("done\n");

  return 0;
}
