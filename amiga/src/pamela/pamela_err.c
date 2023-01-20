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
    case PAMELA_ERROR_CHANNEL_NOT_ACTIVE:
      return "channel not active";
    case PAMELA_ERROR_CHANNEL_STATE:
      return "invalid channel state";
    case PAMELA_ERROR_MSG_TOO_LARGE:
      return "message too large";
    case PAMELA_ERROR_CHANNEL_NOT_FOUND:
      return "channel not found";
    case PAMELA_ERROR_ALREADY_READING:
      return "already reading";
    case PAMELA_ERROR_ALREADY_WRITING:
      return "already writing";
    case PAMELA_ERROR_READ_FAILED:
      return "read failed";
    case PAMELA_ERROR_WRITE_FAILED:
      return "write failed";
    case PAMELA_ERROR_INVALID_MTU:
      return "invalid mtu";
    case PAMELA_ERROR_ODD_MTU:
      return "odd mtu";
    case PAMELA_ERROR_NO_HANDLE:
      return "no handle";
    case PAMELA_ERROR_WIRE:
      return "wire error";
    default:
      return "unkown";
  }
}
