#ifndef PALOMA_LIB_H
#define PALOMA_LIB_H

#include "pamlib.h"
#include "paloma/types.h"

/* Errors extend the Pamela errors */

#define PALOMA_OK                     PAMELA_OK
#define PALOMA_ERROR_NO_CMD           (PAMELA_ERROR_CUSTOM)
#define PALOMA_ERROR_CMD_TOO_LARGE    (PAMELA_ERROR_CUSTOM-1)
#define PALOMA_ERROR_CMD_TOO_SHORT    (PAMELA_ERROR_CUSTOM-2)
#define PALOMA_ERROR_WRONG_CMD        (PAMELA_ERROR_CUSTOM-3)
#define PALOMA_ERROR_WRONG_LEN        (PAMELA_ERROR_CUSTOM-4)
#define PALOMA_ERROR_WRONG_TYPE       (PAMELA_ERROR_CUSTOM-5)
/* last error is start of the remote errors.
   the external status is subtracted from this value
 */
#define PALOMA_ERROR_REMOTE           (PAMELA_ERROR_CUSTOM-6)

/* --- Types --- */

struct paloma_handle;
typedef struct paloma_handle paloma_handle_t;

/* --- API --- */

/* setup paloma instance */
paloma_handle_t *paloma_init(struct Library *SysBase, pamlib_handle_t *ph,
                             UWORD port, int *error);
/* shutdown paloma instance */
int paloma_exit(paloma_handle_t *ph);

/* --- param functions --- */

/* reset all parameters to their factory defaults */
int paloma_param_all_reset(paloma_handle_t *ph);
/* load all parameters from flash storage */
int paloma_param_all_load(paloma_handle_t *ph);
/* save all parameters to flash storage */
int paloma_param_all_save(paloma_handle_t *ph);

/* get total numbers of parameters */
int paloma_param_get_total_slots(paloma_handle_t *ph, UBYTE *num_slots);
/* get param info */
int paloma_param_get_info(paloma_handle_t *ph, UBYTE slot, paloma_param_info_t *info);
/* find slot for given id */
int paloma_param_find_slot(paloma_handle_t *ph, UBYTE id, UBYTE *slot);
/* get param value found in slot */
int paloma_param_get_value(paloma_handle_t *ph, UBYTE slot, UBYTE type, UBYTE *size, UBYTE *data);
/* set param value found in slot */
int paloma_param_set_value(paloma_handle_t *ph, UBYTE slot, UBYTE type, UBYTE size, UBYTE *data);
/* get param value found in slot */
int paloma_param_default_value(paloma_handle_t *ph, UBYTE slot, UBYTE type, UBYTE *size, UBYTE *data);
/* reset param to default value */
int paloma_param_reset(paloma_handle_t *ph, UBYTE slot);

/* helper */
int paloma_param_get_ubyte(paloma_handle_t *ph, UBYTE slot, UBYTE *data);
int paloma_param_set_ubyte(paloma_handle_t *ph, UBYTE slot, UBYTE data);
int paloma_param_default_ubyte(paloma_handle_t *ph, UBYTE slot, UBYTE *data);

int paloma_param_get_uword(paloma_handle_t *ph, UBYTE slot, UWORD *data);
int paloma_param_set_uword(paloma_handle_t *ph, UBYTE slot, UWORD data);
int paloma_param_default_uword(paloma_handle_t *ph, UBYTE slot, UWORD *data);

int paloma_param_get_ulong(paloma_handle_t *ph, UBYTE slot, ULONG *data);
int paloma_param_set_ulong(paloma_handle_t *ph, UBYTE slot, ULONG data);
int paloma_param_default_ulong(paloma_handle_t *ph, UBYTE slot, ULONG *data);

int paloma_param_get_buffer(paloma_handle_t *ph, UBYTE slot, UBYTE *data, UBYTE *size);
int paloma_param_set_buffer(paloma_handle_t *ph, UBYTE slot, UBYTE *data, UBYTE size);
int paloma_param_default_buffer(paloma_handle_t *ph, UBYTE slot, UBYTE *data, UBYTE *size);

#endif
