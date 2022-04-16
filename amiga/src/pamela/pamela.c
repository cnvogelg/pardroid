#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "types.h"
#include "arch.h"

#include "proto_io.h"
#include "pamela.h"
#include "pamela_int.h"

static int read_devinfo(pamela_handle_t *ph)
{
  proto_handle_t *proto = ph->proto;
  pamela_devinfo_t *info = &ph->devinfo;
  int res;

  // firmware id
  res = proto_dev_get_fw_id(proto, &info->firmware_id);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // firmware version
  res = proto_dev_get_fw_version(proto, &info->firmware_version);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // machtag
  res = proto_dev_get_mach_tag(proto, &info->mach_tag);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // default_mtu
  res = proto_io_get_default_mtu(proto, &info->default_mtu);
  if(res != PROTO_RET_OK) {
    return res;
  }

  // max channels
  res = proto_io_get_max_channels(proto, &info->max_channels);
  if(res != PROTO_RET_OK) {
    return res;
  }

  return PROTO_RET_OK;
}

pamela_handle_t *pamela_init(struct Library *SysBase, int *error)
{
  // create handle
  pamela_handle_t *ph = AllocMem(sizeof(pamela_handle_t), MEMF_CLEAR | MEMF_PUBLIC);
  if(ph == NULL) {
    *error = PAMELA_ERROR_NO_MEM;
    return NULL;
  }

  ph->sys_base = SysBase;

  // open env
  int res = 0;
  ph->proto_env = proto_env_init(SysBase, &res);
  if(ph->proto_env == NULL) {
    FreeMem(ph, sizeof(pamela_handle_t));
    *error = PAMELA_ERROR_INIT_ENV;
    return NULL;
  }

  // open handle
  ph->proto = proto_io_init(ph->proto_env);
  if(ph->proto == NULL) {
    proto_env_exit_events(ph->proto_env);
    proto_env_exit(ph->proto_env);
    FreeMem(ph, sizeof(pamela_handle_t));
    *error = PAMELA_ERROR_INIT_PROTO;
    return NULL;
  }

  // init events (after reset of device)
  res = proto_env_init_events(ph->proto_env);
  if(res != PROTO_ENV_OK) {
    proto_env_exit(ph->proto_env);
    FreeMem(ph, sizeof(pamela_handle_t));
    *error = PAMELA_ERROR_INIT_ENV;
    return NULL;
  }

  // setup token
  ph->token = 0xbabe;
  res = proto_dev_set_driver_token(ph->proto, ph->token);
  if(res != PROTO_RET_OK) {
    proto_io_exit(ph->proto);
    proto_env_exit_events(ph->proto_env);
    proto_env_exit(ph->proto_env);
    FreeMem(ph, sizeof(pamela_handle_t));
    *error = pamela_map_proto_error(res);
    return NULL;
  }

  // get device info
  res = read_devinfo(ph);
  if(res != PROTO_RET_OK) {
    proto_io_exit(ph->proto);
    proto_env_exit_events(ph->proto_env);
    proto_env_exit(ph->proto_env);
    FreeMem(ph, sizeof(pamela_handle_t));
    *error = pamela_map_proto_error(res);
    return NULL;
  }

  pamela_channels_init(ph);

  return ph;
}

#undef SysBase
#define SysBase ph->sys_base

void pamela_exit(pamela_handle_t *ph)
{
  if(ph == NULL) {
    return;
  }

  proto_env_exit_events(ph->proto_env);
  proto_io_exit(ph->proto);
  proto_env_exit(ph->proto_env);

  FreeMem(ph, sizeof(pamela_handle_t));
}

timer_handle_t *pamela_get_timer(pamela_handle_t *ph)
{
  if(ph->proto_env == NULL) {
    return NULL;
  }
  return proto_env_get_timer(ph->proto_env);
}

void pamela_devinfo(pamela_handle_t *ph, pamela_devinfo_t *info)
{
  CopyMem(&ph->devinfo, info, sizeof(pamela_devinfo_t));
}

int pamela_get_max_channels(pamela_handle_t *ph)
{
  return ph->devinfo.max_channels;
}

static int get_event_mask(pamela_handle_t *ph, UWORD *events)
{
  int res = proto_io_get_event_mask(ph->proto, events);
  if(res != PROTO_RET_OK) {
    return pamela_map_proto_error(res);
  }
  return PAMELA_OK;
}

/* update pamela status after an IRQ or by polling */
int pamela_event_update(pamela_handle_t *ph, UWORD *event_mask)
{
  UWORD events;
  int res = get_event_mask(ph, &events);
  if(res != PAMELA_OK) {
    return res;
  }

  if(event_mask != NULL) {
    *event_mask = events;
  }

  for(int i=0;i<PROTO_IO_NUM_CHANNELS;i++) {
    UWORD mask = 1 << i;
    if((events & mask) == mask) {
      res = pamela_update(&ph->channels[i]);
      if(res != PAMELA_OK) {
        return res;
      }
    }
  }

  return PAMELA_OK;
}

int pamela_event_wait(pamela_handle_t *ph,
                      ULONG timeout_s, ULONG timeout_us,
                      ULONG *extra_sigmask)
{
  ULONG mask_timeout = proto_env_get_timer_sigmask(ph->proto_env);
  ULONG mask_events  = proto_env_get_trigger_sigmask(ph->proto_env);
  ULONG extra = 0;
  if(extra_sigmask != NULL) {
    extra = *extra_sigmask;
  }

  ULONG got_mask = proto_env_wait_event(ph->proto_env, timeout_s, timeout_us, extra);

  int result = 0;

  // extra sigmask?
  if((got_mask & extra) != 0) {
    *extra_sigmask = got_mask & extra;
    result |= PAMELA_WAIT_SIGMASK;
  }

  // timer?
  if((got_mask & mask_timeout) != 0) {
    result |= PAMELA_WAIT_TIMEOUT;
  }

  // events?
  if((got_mask & mask_events) != 0) {
    result |= PAMELA_WAIT_EVENT;
  }

  return result;
}
