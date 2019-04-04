#include "types.h"
#include "arch.h"
#include "autoconf.h"

#include "knok.h"
#include "proto.h"

void pamela_init(void)
{
  // wait for knockin seq
  knok_main();

  // init proto
  proto_init();

  // receive first action (reset or bootloader)
  proto_first();
}

void pamela_handle(void)
{
  proto_handle();
}
