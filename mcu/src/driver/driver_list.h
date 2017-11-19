#ifndef DRIVER_LIST_H
#define DRIVER_LIST_H

#include "autoconf.h"

#include "drv_null.h"

#ifdef CONFIG_DRIVER_ENC28J60
#include "drv_enc28j60.h"
#endif

#ifdef CONFIG_DRIVER_SDCARD
#include "drv_sdcard.h"
#endif

#endif
