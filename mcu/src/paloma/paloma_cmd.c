#include "types.h"
#include "arch.h"
#include "autoconf.h"

#define DEBUG CONFIG_DEBUG_PALOMA

#include "debug.h"

#include "util.h"
#include "paloma_cmd.h"
#include "paloma_api.h"
#include "paloma/wire.h"
#include "paloma/types.h"

static u08 find_slot(u08 id)
{
  u08 num = paloma_api_param_get_total_slots();
  for(u08 slot = 0; slot < num; slot++) {
    u08 slot_id = paloma_api_param_get_id(slot);
    if(slot_id == id) {
      return slot;
    }
  }
  return PALOMA_WIRE_INVALID_SLOT;
}

static u08 get_value(u08 slot, u08 *payload, u08 *rsize, u08 def)
{
  u08 *data = payload + 2;
  u08 type = paloma_api_param_get_type(slot);
  u08 size = 0;
  switch(type) {
    case PALOMA_TYPE_UBYTE:
      {
        size = PALOMA_TYPE_SIZE_UBYTE;
        *data = paloma_api_param_get_ubyte(slot, def);
        break;
      }
    case PALOMA_TYPE_UWORD:
      {
        size = PALOMA_TYPE_SIZE_UWORD;
        u16 val = paloma_api_param_get_uword(slot, def);
        util_put_uword(val, data);
        break;
      }
    case PALOMA_TYPE_ULONG:
      {
        size = PALOMA_TYPE_SIZE_ULONG;
        u32 val = paloma_api_param_get_ulong(slot, def);
        util_put_ulong(val, data);
        break;
      }
    case PALOMA_TYPE_BUFFER:
      {
        size = paloma_api_param_get_buffer(slot, data);
        break;
      }
    default:
      return PALOMA_WIRE_STATUS_WRONG_TYPE;
  }
  payload[0] = type;
  payload[1] = size;
  *rsize = size;
  return PALOMA_WIRE_STATUS_OK;
}

static u08 set_value(u08 slot, u08 *payload, u08 payload_size, u08 *rsize)
{
  u08 *data = payload + 2;

  u08 type = payload[0];
  u08 size = payload[1];

  if(payload_size != (size + 2)) {
    return PALOMA_WIRE_STATUS_DATA_WRONG_SIZE;
  }

  // check type
  if(type != paloma_api_param_get_type(slot)) {
    return PALOMA_WIRE_STATUS_WRONG_TYPE;
  }

  switch(type) {
    case PALOMA_TYPE_UBYTE:
      {
        if(size != PALOMA_TYPE_SIZE_UBYTE) {
          return PALOMA_WIRE_STATUS_WRONG_SIZE;
        }
        paloma_api_param_set_ubyte(slot, *data);
        break;
      }
    case PALOMA_TYPE_UWORD:
      {
        if(size != PALOMA_TYPE_SIZE_UWORD) {
          return PALOMA_WIRE_STATUS_WRONG_SIZE;
        }
        u16 val = util_get_uword(data);
        paloma_api_param_set_uword(slot, val);
        break;
      }
    case PALOMA_TYPE_ULONG:
      {
        if(size != PALOMA_TYPE_SIZE_ULONG) {
          return PALOMA_WIRE_STATUS_WRONG_SIZE;
        }
        u32 val = util_get_ulong(data);
        paloma_api_param_set_ulong(slot, val);
        break;
      }
    case PALOMA_TYPE_BUFFER:
      {
        u08 min, max;
        paloma_api_param_get_min_max_bytes(slot, &min, &max);
        if((size < min) || (size > max)) {
          return PALOMA_WIRE_STATUS_WRONG_SIZE;
        }
        paloma_api_param_set_buffer(slot, data, size);
        break;
      }
    default:
      return PALOMA_WIRE_STATUS_WRONG_TYPE;
  }
  payload[0] = type;
  payload[1] = size;
  *rsize = size;
  return PALOMA_WIRE_STATUS_OK;

}

static u08 reset_value(u08 slot, u08 *payload)
{
  u08 size = 0;
  u08 res = get_value(slot, payload, &size, 1);
  if(res !=PALOMA_WIRE_STATUS_OK) {
    return res;
  }
  u08 dummy = 0;
  return set_value(slot, payload, size, &dummy);
}

void paloma_cmd_handle(u08 *buf, u08 buf_size, u08 *ret_size)
{
  u08 cmd = buf[0];
  u08 status = PALOMA_WIRE_STATUS_OK;
  u08 rsize = 0;
  u08 *payload = &buf[4];
  u08 tsize = buf[2];

  u08 pkt_size = tsize + PALOMA_WIRE_HEADER_SIZE;
  if((pkt_size & 1)==1) {
    pkt_size++;
  }

  DNL; DS("paloma_cmd="); DB(cmd); DC('#'); DB(buf_size); DC('+'); DB(tsize);
  DC('p'); DB(pkt_size); DC('{'); DNL;

  // check size
  if((buf_size < PALOMA_WIRE_HEADER_SIZE) || (buf_size != pkt_size)) {
    status = PALOMA_WIRE_STATUS_DATA_WRONG_SIZE;
  }
  else {
    switch(cmd) {
      case PALOMA_WIRE_CMD_PARAM_ALL_RESET:
        if(tsize == 0) {
          paloma_api_param_all_reset();
        } else {
          status = PALOMA_WIRE_STATUS_DATA_WRONG_SIZE;
        }
        break;

      case PALOMA_WIRE_CMD_PARAM_ALL_LOAD:
        if(tsize == 0) {
          paloma_api_param_all_load();
        } else {
          status = PALOMA_WIRE_STATUS_DATA_WRONG_SIZE;
        }
        break;

      case PALOMA_WIRE_CMD_PARAM_ALL_SAVE:
        if(tsize == 0) {
          paloma_api_param_all_save();
        } else {
          status = PALOMA_WIRE_STATUS_DATA_WRONG_SIZE;
        }
        break;

      case PALOMA_WIRE_CMD_PARAM_GET_TOTAL_SLOTS:
        if(tsize == 0) {
          *payload = paloma_api_param_get_total_slots();
          rsize = 1;
        } else {
          status = PALOMA_WIRE_STATUS_DATA_WRONG_SIZE;
        }
        break;

      case PALOMA_WIRE_CMD_PARAM_GET_INFO:
        if(tsize == 1) {
          u08 slot = payload[0];
          if(slot < paloma_api_param_get_total_slots()) {
            paloma_param_info_t *info = (paloma_param_info_t *)payload;
            paloma_api_param_get_info(slot, info);
            rsize = sizeof(paloma_param_info_t);
          } else {
            status = PALOMA_WIRE_STATUS_WRONG_SLOT;
          }
        } else {
          status = PALOMA_WIRE_STATUS_DATA_WRONG_SIZE;
        }
        break;

      case PALOMA_WIRE_CMD_PARAM_FIND_SLOT:
        if(tsize == 1) {
          u08 id = payload[0];
          *payload = find_slot(id);
          rsize = 1;
        } else {
          status = PALOMA_WIRE_STATUS_DATA_WRONG_SIZE;
        }
        break;

      case PALOMA_WIRE_CMD_PARAM_GET_VALUE:
        if(tsize == 1) {
          u08 slot = payload[0];
          if(slot < paloma_api_param_get_total_slots()) {
            status = get_value(slot, payload, &rsize, 0);
          } else {
            status = PALOMA_WIRE_STATUS_WRONG_SLOT;
          }
        } else {
          status = PALOMA_WIRE_STATUS_DATA_WRONG_SIZE;
        }
        break;

      case PALOMA_WIRE_CMD_PARAM_SET_VALUE:
        if(tsize > 1) {
          u08 slot = payload[0];
          if(slot < paloma_api_param_get_total_slots()) {
            status = set_value(slot, payload, tsize, &rsize);
          } else {
            status = PALOMA_WIRE_STATUS_WRONG_SLOT;
          }
        } else {
          status = PALOMA_WIRE_STATUS_DATA_WRONG_SIZE;
        }
        break;

      case PALOMA_WIRE_CMD_PARAM_GET_DEFAULT:
        if(tsize == 1) {
          u08 slot = payload[0];
          if(slot < paloma_api_param_get_total_slots()) {
            status = get_value(slot, payload, &rsize, 1);
          } else {
            status = PALOMA_WIRE_STATUS_WRONG_SLOT;
          }
        } else {
          status = PALOMA_WIRE_STATUS_DATA_WRONG_SIZE;
        }
        break;

      case PALOMA_WIRE_CMD_PARAM_RESET:
        if(tsize == 1) {
          u08 slot = payload[0];
          if(slot < paloma_api_param_get_total_slots()) {
            status = reset_value(slot, payload);
          } else {
            status = PALOMA_WIRE_STATUS_WRONG_SLOT;
          }
        } else {
          status = PALOMA_WIRE_STATUS_DATA_WRONG_SIZE;
        }
        break;

      default:
        status = PALOMA_WIRE_STATUS_NO_CMD;
        break;
    }
  }

  /* store status and return result packet size */
  buf[1] = status;
  buf[2] = rsize;
  buf[3] = 0;

  // pad to word
  if((rsize & 1)==1) {
    rsize++;
  }

  *ret_size = rsize + PALOMA_WIRE_HEADER_SIZE;

  DS("}:"); DB(status); DC('#'); DB(rsize); DNL;
}
