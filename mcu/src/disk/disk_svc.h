/* disk_svc.h - disk storage service in pamela */

#ifndef DISK_SVC_H
#define DISK_SVC_H

#include "pamela_req.h"
#include "pamela.h"

// disk control pamela_req service
REQ_HANDLER_DECLARE(disk_svc_ctl);
// disk data pamela service
HANDLER_DECLARE(disk_svc_data);

#endif
