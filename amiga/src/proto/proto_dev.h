#ifndef PROTO_DEV_H
#define PROTO_DEV_H

#include "proto_env.h"
#include "proto_atom.h"

/* init/exit of dev mode */
extern proto_handle_t *proto_dev_init(proto_env_handle_t *penv);
extern void proto_dev_exit(proto_handle_t *ph);

/* device commands */
extern int proto_dev_action_ping(proto_handle_t *ph);
extern int proto_dev_action_bootloader(proto_handle_t *ph);
extern int proto_dev_action_reset(proto_handle_t *ph);
extern int proto_dev_action_knok(proto_handle_t *ph);

/* device parameters */
extern int proto_dev_get_fw_id(proto_handle_t *ph, UWORD *result);
extern int proto_dev_get_fw_version(proto_handle_t *ph, UWORD *result);
extern int proto_dev_get_mach_tag(proto_handle_t *ph, UWORD *result);

/* handle driver token */
extern int proto_dev_get_driver_token(proto_handle_t *ph, UWORD *result);
extern int proto_dev_set_driver_token(proto_handle_t *ph, UWORD token);

#endif
