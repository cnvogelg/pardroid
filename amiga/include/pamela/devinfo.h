#ifndef PAMELA_COMMON_H
#define PAMELA_COMMON_H

#include <exec/exec.h>

/* device information */
struct pamela_devinfo {
  UWORD     firmware_id;
  UWORD     firmware_version;
  UWORD     mach_tag;
  UWORD     default_mtu;
  UWORD     max_channels;
};
typedef struct pamela_devinfo pamela_devinfo_t;

#endif
