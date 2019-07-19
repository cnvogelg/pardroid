#include "types.h"
#include "autoconf.h"

#include "proto.h"
#include "proto_low.h"

void proto_api_read_block_spi(u16 num_words) 
{
  proto_low_read_block_spi(num_words);
}

void proto_api_write_block_spi(u16 num_words)
{
  proto_low_write_block_spi(num_words);
}
