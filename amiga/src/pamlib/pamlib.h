#ifndef PAMLIB_H
#define PAMLIB_H

#include <exec/exec.h>

#include "pamela/devinfo.h"
#include "pamela/error.h"

/* ----- Types ----- */

struct pamlib_handle;
typedef struct pamlib_handle pamlib_handle_t;

struct pamlib_channel;
typedef struct pamlib_channel pamlib_channel_t;

/* ----- API ----- */

/* decode error */
const char *pamela_perror(int res);

/* setup pamlib and underlying device */
pamlib_handle_t *pamlib_init(struct Library *SysBase, int *error, char *dev_name);
/* shutdown pamlib */
void pamlib_exit(pamlib_handle_t *ph);

/* clone handle to work with it in another task */
pamlib_handle_t *pamlib_clone(pamlib_handle_t *ph);

/* fill devinfo */
int pamlib_devinfo(pamlib_handle_t *ph, pamela_devinfo_t *di);

/* open a channel */
pamlib_channel_t *pamlib_open(pamlib_handle_t *ph, UWORD port, int *error);
/* close a channel */
int pamlib_close(pamlib_channel_t *pc);
/* reset a channel */
int pamlib_reset(pamlib_channel_t *pc);

/* read data */
int pamlib_read(pamlib_channel_t *pc, UBYTE *data, UWORD size);
/* write data */
int pamlib_write(pamlib_channel_t *pc, UBYTE *data, UWORD size);

/* seek */
int pamlib_seek(pamlib_channel_t *pc, ULONG pos);
/* tell */
int pamlib_tell(pamlib_channel_t *pc, ULONG *pos);

#endif
