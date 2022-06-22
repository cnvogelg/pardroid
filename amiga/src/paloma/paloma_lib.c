#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "types.h"
#include "arch.h"

#include "paloma_lib.h"
#include "pamela/error.h"
#include "paloma/wire.h"

struct paloma_handle {
  struct Library     *sys_base;
  pamlib_handle_t    *pam;
  pamlib_channel_t   *channel;
  UWORD               port;
  UBYTE              *payload;
  UBYTE               data_buffer[PALOMA_WIRE_MAX_PACKET_SIZE];
};

paloma_handle_t *paloma_init(struct Library *SysBase, pamlib_handle_t *pam,
                             UWORD port, int *error)
{
  // try to open channel to port
  pamlib_channel_t *channel = pamlib_open(pam, port, error);
  if(channel == NULL) {
    return NULL;
  }

  // create handle
  paloma_handle_t *ph = AllocMem(sizeof(paloma_handle_t), MEMF_CLEAR | MEMF_PUBLIC);
  if(ph == NULL) {
    *error = PAMELA_ERROR_NO_MEM;
    return NULL;
  }

  ph->sys_base = SysBase;
  ph->pam = pam;
  ph->port = port;
  ph->channel = channel;

  /* pointer to begin of payload */
  ph->payload = &ph->data_buffer[PALOMA_WIRE_HEADER_SIZE];

  return ph;
}

#undef SysBase
#define SysBase ph->sys_base

int paloma_exit(paloma_handle_t *ph)
{
  if(ph == NULL) {
    return PALOMA_OK;
  }

  int result = pamlib_close(ph->channel);

  FreeMem(ph, sizeof(paloma_handle_t));

  return result;
}

static int transfer_cmd(paloma_handle_t *ph, UBYTE cmd, UWORD tx_len, UWORD *rx_len)
{
  if(tx_len > PALOMA_WIRE_MAX_PAYLOAD_SIZE) {
    return PALOMA_ERROR_CMD_TOO_LARGE;
  }

  /* first send buffer with command */
  ph->data_buffer[0] = cmd;
  ph->data_buffer[1] = 0;
  ph->data_buffer[2] = (UBYTE)tx_len;
  ph->data_buffer[3] = 0;

  UWORD pkt_len = tx_len + PALOMA_WIRE_HEADER_SIZE;

  int res = pamlib_write(ph->channel, ph->data_buffer, pkt_len);
  if(res != PAMELA_OK) {
    return res;
  }

  /* receive result */
  res = pamlib_read(ph->channel, ph->data_buffer, PALOMA_WIRE_MAX_PACKET_SIZE);
  if(res < 0) {
    return res;
  }
  /* no header? */
  if(res < PALOMA_WIRE_HEADER_SIZE) {
    return PALOMA_ERROR_CMD_TOO_SHORT;
  }
  /* check command */
  if(ph->data_buffer[0] != cmd) {
    return PALOMA_ERROR_WRONG_CMD;
  }
  /* check len */
  pkt_len = ph->data_buffer[2] + PALOMA_WIRE_HEADER_SIZE;
  if(pkt_len != res) {
    return PALOMA_ERROR_WRONG_LEN;
  }
  /* check status */
  int status = ph->data_buffer[1];
  if(status == 0) {
    if(rx_len != NULL) {
      *rx_len = ph->data_buffer[2];
    }
    return PALOMA_OK;
  } else {
    /* remote error is mapped to error values
       starting at PAMELA_ERROR_REMOTE and descending */
    return PALOMA_ERROR_REMOTE - status;
  }
}

int paloma_param_all_reset(paloma_handle_t *ph)
{
  return transfer_cmd(ph, PALOMA_WIRE_CMD_PARAM_ALL_RESET, 0, NULL);
}

int paloma_param_all_load(paloma_handle_t *ph)
{
  return transfer_cmd(ph, PALOMA_WIRE_CMD_PARAM_ALL_LOAD, 0, NULL);
}

int paloma_param_all_save(paloma_handle_t *ph)
{
  return transfer_cmd(ph, PALOMA_WIRE_CMD_PARAM_ALL_SAVE, 0, NULL);
}

static UWORD get_word(paloma_handle_t *ph, int offset)
{
  UBYTE *payload = ph->payload + offset;
  return payload[0] | (UWORD)payload[1] << 8;
}

static void put_word(paloma_handle_t *ph, int offset, UWORD val)
{
  UBYTE *payload = ph->payload + offset;
  payload[0] = (UBYTE)(val & 0xff);
  payload[1] = (UBYTE)(val >> 8);
}

int paloma_param_get_total_slots(paloma_handle_t *ph, UBYTE *num_slots)
{
  UWORD rx_len = 0;

  int res = transfer_cmd(ph, PALOMA_WIRE_CMD_PARAM_GET_TOTAL_SLOTS, 0, &rx_len);
  if(res != PALOMA_OK) {
    return res;
  }

  // out
  if(rx_len != 1) {
    return PALOMA_ERROR_WRONG_LEN;
  }
  *num_slots = ph->payload[0];
  return PALOMA_OK;
}

int paloma_param_get_info(paloma_handle_t *ph, UBYTE slot, paloma_param_info_t *info)
{
  UWORD rx_len = 0;

  // in
  ph->payload[0] = slot;

  int res = transfer_cmd(ph, PALOMA_WIRE_CMD_PARAM_GET_INFO, 1, &rx_len);
  if(res != PALOMA_OK) {
    return res;
  }
  if(rx_len != sizeof(paloma_param_info_t)) {
    return PALOMA_ERROR_WRONG_LEN;
  }

  // out
  CopyMem(ph->payload, info, sizeof(paloma_param_info_t));
  return PALOMA_OK;
}

int paloma_param_find_slot(paloma_handle_t *ph, UBYTE id, UBYTE *slot)
{
  UWORD rx_len = 0;

  // in
  ph->payload[0] = id;

  int res = transfer_cmd(ph, PALOMA_WIRE_CMD_PARAM_FIND_SLOT, 1, &rx_len);
  if(res != PALOMA_OK) {
    return res;
  }
  if(rx_len != 1) {
    return PALOMA_ERROR_WRONG_LEN;
  }

  // out
  *slot = ph->payload[0];
  return PALOMA_OK;
}

int paloma_param_get_value(paloma_handle_t *ph, UBYTE slot, UBYTE type,
                           UBYTE *size, UBYTE *data)
{
  UWORD rx_len = 0;

  // in
  ph->payload[0] = slot;

  int res = transfer_cmd(ph, PALOMA_WIRE_CMD_PARAM_GET_VALUE, 1, &rx_len);
  if(res != PALOMA_OK) {
    return res;
  }

  // out
  UBYTE got_type = ph->payload[0];
  if(got_type != type) {
    return PALOMA_ERROR_WRONG_TYPE;
  }

  UBYTE got_size = ph->payload[1];
  CopyMem(&ph->payload[2], data, got_size);
  if(size != NULL) {
    *size = got_size;
  }

  return PALOMA_OK;
}

int paloma_param_get_default(paloma_handle_t *ph, UBYTE slot, UBYTE type,
                             UBYTE *size, UBYTE *data)
{
  UWORD rx_len = 0;

  // in
  ph->payload[0] = slot;

  int res = transfer_cmd(ph, PALOMA_WIRE_CMD_PARAM_GET_DEFAULT, 1, &rx_len);
  if(res != PALOMA_OK) {
    return res;
  }

  // out
  UBYTE got_type = ph->payload[0];
  if(got_type != type) {
    return PALOMA_ERROR_WRONG_TYPE;
  }

  UBYTE got_size = ph->payload[1];
  CopyMem(&ph->payload[2], data, got_size);
  if(size != NULL) {
    *size = got_size;
  }

  return PALOMA_OK;
}

int paloma_param_set_value(paloma_handle_t *ph, UBYTE slot, UBYTE type,
                           UBYTE size, UBYTE *data)
{
  UWORD rx_len = 0;

  // in
  ph->payload[0] = slot;
  ph->payload[1] = type;
  ph->payload[2] = size;
  CopyMem(data, &ph->payload[3], size);
  UBYTE tx_len = 3 + size;

  int res = transfer_cmd(ph, PALOMA_WIRE_CMD_PARAM_SET_VALUE, tx_len, &rx_len);
  if(res != PALOMA_OK) {
    return res;
  }
  if(rx_len != 1) {
    return PALOMA_ERROR_WRONG_LEN;
  }

  // out
  UBYTE got_type = ph->payload[0];
  if(got_type != type) {
    return PALOMA_ERROR_WRONG_TYPE;
  }

  return PALOMA_OK;
}

int paloma_param_reset(paloma_handle_t *ph, UBYTE slot)
{
  UWORD rx_len = 0;

  // in
  ph->payload[0] = slot;

  int res = transfer_cmd(ph, PALOMA_WIRE_CMD_PARAM_RESET, 1, &rx_len);
  if(res != PALOMA_OK) {
    return res;
  }
  if(rx_len != 0) {
    return PALOMA_ERROR_WRONG_LEN;
  }

  return PALOMA_OK;
}

int paloma_param_get_ubyte_id(paloma_handle_t *ph, UBYTE id, UBYTE *data)
{
  UBYTE slot;
  int res = paloma_param_find_slot(ph, id, &slot);
  if(res != PALOMA_OK) {
    return res;
  }
  return paloma_param_get_value(ph, slot, PALOMA_TYPE_UBYTE, NULL, data);
}

int paloma_param_set_ubyte_id(paloma_handle_t *ph, UBYTE id, UBYTE data)
{
  UBYTE slot;
  int res = paloma_param_find_slot(ph, id, &slot);
  if(res != PALOMA_OK) {
    return res;
  }
  return paloma_param_set_value(ph, slot, PALOMA_TYPE_UBYTE, NULL, &data);
}

int paloma_param_get_uword_id(paloma_handle_t *ph, UBYTE id, UWORD *data)
{
  UBYTE slot;
  int res = paloma_param_find_slot(ph, id, &slot);
  if(res != PALOMA_OK) {
    return res;
  }
  return paloma_param_get_value(ph, slot, PALOMA_TYPE_UWORD, NULL, (UBYTE *)data);
}

int paloma_param_set_uword_id(paloma_handle_t *ph, UBYTE id, UWORD data)
{
  UBYTE slot;
  int res = paloma_param_find_slot(ph, id, &slot);
  if(res != PALOMA_OK) {
    return res;
  }
  return paloma_param_set_value(ph, slot, PALOMA_TYPE_UWORD, NULL, (UBYTE *)&data);
}

int paloma_param_get_ulong_id(paloma_handle_t *ph, UBYTE id, ULONG *data)
{
  UBYTE slot;
  int res = paloma_param_find_slot(ph, id, &slot);
  if(res != PALOMA_OK) {
    return res;
  }
  return paloma_param_get_value(ph, slot, PALOMA_TYPE_ULONG, NULL, (UBYTE *)data);
}

int paloma_param_set_ulong_id(paloma_handle_t *ph, UBYTE id, ULONG data)
{
  UBYTE slot;
  int res = paloma_param_find_slot(ph, id, &slot);
  if(res != PALOMA_OK) {
    return res;
  }
  return paloma_param_set_value(ph, slot, PALOMA_TYPE_ULONG, NULL, (UBYTE *)&data);
}

int paloma_param_get_ip_addr_id(paloma_handle_t *ph, UBYTE id, paloma_param_ip_addr_t *data)
{
  UBYTE slot;
  int res = paloma_param_find_slot(ph, id, &slot);
  if(res != PALOMA_OK) {
    return res;
  }
  return paloma_param_get_value(ph, slot, PALOMA_TYPE_IP_ADDR, NULL, (UBYTE *)data);
}

int paloma_param_set_ip_addr_id(paloma_handle_t *ph, UBYTE id, paloma_param_ip_addr_t *data)
{
  UBYTE slot;
  int res = paloma_param_find_slot(ph, id, &slot);
  if(res != PALOMA_OK) {
    return res;
  }
  return paloma_param_set_value(ph, slot, PALOMA_TYPE_IP_ADDR, NULL, (UBYTE *)data);
}

int paloma_param_get_mac_addr_id(paloma_handle_t *ph, UBYTE id, paloma_param_mac_addr_t *data)
{
  UBYTE slot;
  int res = paloma_param_find_slot(ph, id, &slot);
  if(res != PALOMA_OK) {
    return res;
  }
  return paloma_param_get_value(ph, slot, PALOMA_TYPE_MAC_ADDR, NULL, (UBYTE *)data);
}

int paloma_param_set_mac_addr_id(paloma_handle_t *ph, UBYTE id, paloma_param_mac_addr_t *data)
{
  UBYTE slot;
  int res = paloma_param_find_slot(ph, id, &slot);
  if(res != PALOMA_OK) {
    return res;
  }
  return paloma_param_set_value(ph, slot, PALOMA_TYPE_MAC_ADDR, NULL, (UBYTE *)data);
}

int paloma_param_get_string_id(paloma_handle_t *ph, UBYTE id, paloma_param_string_t *data)
{
  UBYTE slot;
  int res = paloma_param_find_slot(ph, id, &slot);
  if(res != PALOMA_OK) {
    return res;
  }
  return paloma_param_get_value(ph, slot, PALOMA_TYPE_STRING, &data->length, data->data);
}

int paloma_param_set_string_id(paloma_handle_t *ph, UBYTE id, paloma_param_string_t *data)
{
  UBYTE slot;
  int res = paloma_param_find_slot(ph, id, &slot);
  if(res != PALOMA_OK) {
    return res;
  }
  return paloma_param_set_value(ph, slot, PALOMA_TYPE_STRING, data->length, data->data);
}
