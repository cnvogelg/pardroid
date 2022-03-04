#ifndef PAMELA_H
#define PAMELA_H

#include <exec/exec.h>

#include "pamela_defs.h"

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
int pamela_devinfo(pamela_handle_t *ph, pamela_devinfo_t *info);

/* first get event mask and then update all affected channels  */
int pamela_event_update(pamela_handle_t *ph);
/* wait for event, retrieve mask and update affected channels */
int pamela_event_wait(pamela_handle_t *ph,
                      ULONG timeout_s, ULONG timeout_us,
                      ULONG *extra_sigmask);

/* open channel to given service */
pamela_channel_t *pamela_open(pamela_handle_t *ph, UWORD port, int *error);
/* close channel */
int pamela_close(pamela_channel_t *pc);
/* update channel state */
int pamela_update(pamela_channel_t *pc, UWORD *status);

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
