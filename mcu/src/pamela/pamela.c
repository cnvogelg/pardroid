#include "types.h"
#include "arch.h"
#include "autoconf.h"

#include "knok.h"
#include "status.h"
#include "proto.h"

void pamela_init(void)
{
  // wait for knockin seq
  knok_main();

  proto_init(PROTO_STATUS_DETACHED);
  status_init();
}

void pamela_handle(void)
{
  status_handle();
  proto_handle();
}
