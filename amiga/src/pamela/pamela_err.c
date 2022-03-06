#include <proto/exec.h>

#include "proto_atom.h"
#include "pamela.h"
#include "pamela_int.h"

int pamela_map_proto_error(int proto_error)
{
  switch(proto_error) {
    case PROTO_RET_RAK_INVALID:
      return PAMELA_ERROR_PROTO_RAK_INVALID;
    case PROTO_RET_TIMEOUT:
      return PAMELA_ERROR_PROTO_TIMEOUT;
    case PROTO_RET_DEVICE_BUSY:
      return PAMELA_ERROR_PROTO_DEV_BUSY;
    case PROTO_RET_ODD_BLOCK_SIZE:
      return PAMELA_ERROR_PROTO_ODD_SIZE; 
    default:
      return PAMELA_ERROR_UNKNOWN;
  }
}

const char *pamela_perror(int res)
{
  switch(res) {
    case PAMELA_OK:
      return "OK";
    case PAMELA_ERROR_NO_MEM:
      return "no mem";
    case PAMELA_ERROR_INIT_ENV:
      return "init env failed";
    case PAMELA_ERROR_INIT_PROTO:
      return "init proto failed";
    case PAMELA_ERROR_PROTO_RAK_INVALID:
      return "proto: rak invalid";
    case PAMELA_ERROR_PROTO_TIMEOUT:
      return "proto: time out";
    case PAMELA_ERROR_PROTO_DEV_BUSY:
      return "proto: dev busy";
    case PAMELA_ERROR_PROTO_ODD_SIZE:
      return "proto: odd size";
    case PAMELA_ERROR_NO_FREE_CHANNEL:
      return "no free channel";
    case PAMELA_ERROR_DEV_OPEN_FAILED:
      return "dev open failed";
    case PAMELA_ERROR_CHANNEL_NOT_OPEN:
      return "channel not open";
    case PAMELA_ERROR_CHANNEL_RESET:
      return "channel was reset";
    case PAMELA_ERROR_CHANNEL_EOS:
      return "channel reached end";
    case PAMELA_ERROR_CHANNEL_ERROR:
      return "channel error";
    case PAMELA_ERROR_CHANNEL_STATE:
      return "invalid channel state";
    default:
      return "unkown";
  }
}