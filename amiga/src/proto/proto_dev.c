#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "pario.h"
#include "timer.h"
#include "proto_atom.h"
#include "proto_dev.h"
#include "proto_dev_shared.h"
#include "proto_atom.h"

proto_handle_t *proto_dev_init(proto_env_handle_t *penv)
{
  proto_handle_t *ph = proto_atom_init(penv);
  if(ph == NULL) {
    return NULL;
  }

  // trigger reset to enter main loop of device
  int res = proto_dev_action_reset(ph);
  if(res != PROTO_RET_OK) {
    proto_atom_exit(ph);
    return NULL;
  }

  return ph;
}

void proto_dev_exit(proto_handle_t *ph)
{
  // trigger knok mode to return to save state of device
  proto_dev_action_knok(ph);

  proto_atom_exit(ph);
}

int proto_dev_action_ping(proto_handle_t *ph)
{
  return proto_atom_action(ph, PROTO_DEV_CMD_ACTION_PING);
}

int proto_dev_action_bootloader(proto_handle_t *ph)
{
  return proto_atom_action_no_busy(ph, PROTO_DEV_CMD_ACTION_BOOTLOADER);
}

int proto_dev_action_reset(proto_handle_t *ph)
{
  return proto_atom_action_no_busy(ph, PROTO_DEV_CMD_ACTION_RESET);
}

int proto_dev_action_knok(proto_handle_t *ph)
{
  return proto_atom_action_no_busy(ph, PROTO_DEV_CMD_ACTION_KNOK);
}

/* device parameters */
int proto_dev_get_fw_id(proto_handle_t *ph, UWORD *result)
{
  return proto_atom_read_word(ph, PROTO_DEV_CMD_RWORD_FW_ID, result);
}

int proto_dev_get_fw_version(proto_handle_t *ph, UWORD *result)
{
  return proto_atom_read_word(ph, PROTO_DEV_CMD_RWORD_FW_VERSION, result);
}

int proto_dev_get_mach_tag(proto_handle_t *ph, UWORD *result)
{
  return proto_atom_read_word(ph, PROTO_DEV_CMD_RWORD_MACH_TAG, result);
}

int proto_dev_get_driver_token(proto_handle_t *ph, UWORD *result)
{
  return proto_atom_read_word(ph, PROTO_DEV_CMD_RWORD_DRIVER_TOKEN, result);
}

int proto_dev_set_driver_token(proto_handle_t *ph, UWORD token)
{
  return proto_atom_write_word(ph, PROTO_DEV_CMD_WWORD_DRIVER_TOKEN, token);
}
