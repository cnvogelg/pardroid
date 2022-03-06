#ifndef PAMELA_INT_H
#define PAMELA_INT_H

#include "proto_io_shared.h"

#define CHANNEL_FLAG_EMPTY   0
#define CHANNEL_FLAG_OPEN    1

struct pamela_channel {
  pamela_handle_t    *pamela;
  UBYTE               flags;
  UBYTE               channel_id;
  UWORD               status;
  UWORD               port;
  UWORD               mtu;
  UWORD               read_bytes;
  UWORD               write_bytes;
};

struct pamela_handle {
  proto_env_handle_t *proto_env;
  proto_handle_t     *proto;
  struct Library     *sys_base;
  pamela_devinfo_t    devinfo;
  UWORD               token;
  UWORD               num_channels;

  pamela_channel_t    channels[PROTO_IO_NUM_CHANNELS];
};

extern int pamela_map_proto_error(int proto_error);
extern void pamela_channels_init(pamela_handle_t *ph);


#endif