#ifndef PAMCMD_H
#define PAMCMD_H

#include "pamlib.h"

#define PAMCMD_HEADER_SIZE 4

struct pamcmd {
  UWORD  tx_arg_size;
  UWORD  rx_arg_size;
  UBYTE *arg_buf;
  UBYTE  cmd_id;
  UBYTE  status;
};
typedef struct pamcmd pamcmd_t;

pamcmd_t *pamcmd_open(pamlib_handle_t *ch, UWORD port, int *error);
int pamcmd_close(pamcmd_t *cmd);

int pamcmd_transfer(pamcmd_t *cmd);

#endif
