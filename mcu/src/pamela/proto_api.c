/* implement the proto_api_* and map to channel handler */

#include "types.h"
#include "autoconf.h"
#include "arch.h"

#define DEBUG CONFIG_DEBUG_PROTO

#include "proto_api.h"
#include "channel.h"
#include "fw_info.h"
#include "machtag.h"
#include "status.h"

/* keep a channel index for channel-related global commands */
static u08 channel_id;

void proto_api_action(u08 num)
{
  /* handle channel actions */
  switch (num) {
    case PROTO_ACTION_CHN_OPEN:
      channel_open(channel_id);
      break;
    case PROTO_ACTION_CHN_CLOSE:
      channel_close(channel_id);
      break;
    case PROTO_ACTION_CHN_RESET:
      channel_reset(channel_id);
      break;
  }
}

u16 proto_api_wfunc_read(u08 num)
{
  switch (num) {
    /* firmware info */
    case PROTO_WFUNC_READ_FW_ID:
      return FW_GET_ID();
    case PROTO_WFUNC_READ_FW_VERSION:
      return FW_GET_VERSION();
    case PROTO_WFUNC_READ_MACHTAG:
      return FW_GET_MACHTAG();
    /* read status/error mask */
    case PROTO_WFUNC_READ_STATUS_MASK:
      return status_get_status_mask();
    case PROTO_WFUNC_READ_ERROR_MASK:
      return status_get_error_mask();
    /* channel functions */
    case PROTO_WFUNC_READ_CHN_INDEX:
      return channel_id;
    case PROTO_WFUNC_READ_CHN_MTU:
      return channel_get_mtu(channel_id);
    case PROTO_WFUNC_READ_CHN_ERROR_CODE:
      return channel_get_error_code(channel_id);
    case PROTO_WFUNC_READ_CHN_MODE:
      return channel_get_mode(channel_id);
    case PROTO_WFUNC_READ_CHN_DEF_MTU:
      return channel_get_def_mtu(channel_id);
    default:
      return 0;
  }
}

void proto_api_wfunc_write(u08 num, u16 val)
{
  switch(num) {
    case PROTO_WFUNC_WRITE_CHN_INDEX:
      {
        if(val < PROTO_MAX_CHANNEL) {
          channel_id = val;
        }
      }
      break;
    case PROTO_WFUNC_WRITE_CHN_MTU:
      channel_set_mtu(channel_id, val);
      break;
    default:
      break;
  }
}

u32 proto_api_lfunc_read(u08 num)
{
  switch(num) {
    case PROTO_LFUNC_READ_CHN_OFFSET:
      return channel_get_offset(channel_id);
    default:
      return 0;
  }
}

void proto_api_lfunc_write(u08 num, u32 val)
{
}
