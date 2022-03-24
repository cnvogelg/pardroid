#ifndef PAMELA_H
#define PAMELA_H

#include <exec/exec.h>

#include "pamela_shared.h"
#include "pamela_err.h"

/* wait event result */
#define PAMELA_WAIT_TIMEOUT              1
#define PAMELA_WAIT_SIGMASK              2
#define PAMELA_WAIT_EVENT                4

/* check if read req is done */
static inline int pamela_status_read_ready(UWORD status)
{
  return (status & PAMELA_STATUS_READ_READY) == PAMELA_STATUS_READ_READY;
}

/* check if write req is done */
static inline int pamela_status_write_ready(UWORD status)
{
  return (status & PAMELA_STATUS_WRITE_READY) == PAMELA_STATUS_WRITE_READY;
}

/* device information */
struct pamela_devinfo {
  UWORD     firmware_id;
  UWORD     firmware_version;
  UWORD     mach_tag;
  UWORD     default_mtu;
  UWORD     max_channels;
};
typedef struct pamela_devinfo pamela_devinfo_t;

/* ----- Types ----- */

/* handle for pamela instance */
struct pamela_handle;
typedef struct pamela_handle pamela_handle_t;

/* pamela channel */
struct pamela_channel;
typedef struct pamela_channel pamela_channel_t;

/* ----- API ----- */

/* setup pamela */
pamela_handle_t *pamela_init(struct Library *SysBase, int *error);
/* shutdown pamela */
void pamela_exit(pamela_handle_t *ph);

/* error decoding */
const char *pamela_perror(int res);

/* fill in devinfo struct */
void pamela_devinfo(pamela_handle_t *ph, pamela_devinfo_t *info);
/* return max channels */
int  pamela_get_max_channels(pamela_handle_t *ph);

/* first get event mask and then update all affected channels  */
int pamela_event_update(pamela_handle_t *ph, UWORD *event_mask);
/* wait for event, retrieve mask and update affected channels */
int pamela_event_wait(pamela_handle_t *ph,
                      ULONG timeout_s, ULONG timeout_us,
                      ULONG *extra_sigmask);

/* ----- open/close/reset ----- */
/* open channel to given service */
pamela_channel_t *pamela_open(pamela_handle_t *ph, UWORD port, int *error);
/* close channel */
int pamela_close(pamela_channel_t *pc);
/* reset current channel */
int pamela_reset(pamela_channel_t *pc);

/* ----- update/status ----- */
/* update channel state by querying device */
int pamela_update(pamela_channel_t *pc);
/* return local channel status */
UWORD pamela_status(pamela_channel_t *pc);
/* return id 0..n of channel */
UBYTE pamela_channel_id(pamela_channel_t *pc);

/* ----- get_mtu/set_mtu ----- */
/* return cached mtu value */
UWORD pamela_get_mtu(pamela_channel_t *pc);
/* try to set new mtu value in device */
int pamela_set_mtu(pamela_channel_t *pc, UWORD mtu);

/* ----- seek/tell ----- */
/* seek to position */
int pamela_seek(pamela_channel_t *pc, ULONG pos);
/* get current position */
int pamela_tell(pamela_channel_t *pc, ULONG *pos);

/* ----- read ----- */
/* post read request. give max size */
int pamela_read_request(pamela_channel_t *pc, UWORD size);
/* fetch data of read request. returns number of bytes actually read */
int pamela_read_data(pamela_channel_t *pc, UBYTE *buf);

/* ----- write ----- */
/* post write requst. give max size */
int pamela_write_request(pamela_channel_t *pc, UWORD size);
/* write data and return number of bytes written */
int pamela_write_data(pamela_channel_t *pc, UBYTE *buf);

#endif /* PAMELA_H */
