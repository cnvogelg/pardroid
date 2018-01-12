#ifndef PALOMA_H
#define PALOMA_H

#include "pamela.h"

/* paloma init error codes */
#define PALOMA_OK                 0
#define PALOMA_ERROR_IN_PAMELA    1

struct paloma_handle {
  pamela_handle_t  *pamela;
  int               pamela_error;
};
typedef struct paloma_handle paloma_handle_t;

extern int paloma_init(paloma_handle_t *ph, pamela_handle_t *pm);
extern void paloma_exit(paloma_handle_t *ph);

const char *paloma_perror(int res);

#endif
