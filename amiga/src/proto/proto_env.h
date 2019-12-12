#ifndef PROTO_ENV
#define PROTO_ENV

#include "proto.h"
#include "timer.h"

#define PROTO_ENV_OK                  0
#define PROTO_ENV_ERROR_INIT_PARIO    1
#define PROTO_ENV_ERROR_INIT_TIMER    2
#define PROTO_ENV_ERROR_INIT_PROTO    3
#define PROTO_ENV_ERROR_NO_SIGNAL     4
#define PROTO_ENV_ERROR_ACK_IRQ       5
#define PROTO_ENV_ERROR_TIMER_SIGNAL  6


struct proto_env_handle;
typedef struct proto_env_handle proto_env_handle_t;

proto_env_handle_t *proto_env_init(struct Library *SysBase, int *res);
void proto_env_exit(proto_env_handle_t *ph);

extern const char *proto_env_perror(int res);

proto_handle_t *proto_env_get_proto(proto_env_handle_t *ph);
timer_handle_t *proto_env_get_timer(proto_env_handle_t *ph);

int proto_env_init_events(proto_env_handle_t *ph);
void proto_env_exit_events(proto_env_handle_t *ph);
ULONG proto_env_wait_event(proto_env_handle_t *ph,
                        ULONG timeout_s, ULONG timeout_us, ULONG extra_sigmask);
ULONG proto_env_get_trigger_sigmask(proto_env_handle_t *ph);
ULONG proto_env_get_timer_sigmask(proto_env_handle_t *ph);
UWORD proto_env_get_num_triggers(proto_env_handle_t *ph);
UWORD proto_env_get_num_trigger_signals(proto_env_handle_t *ph);

proto_env_handle_t *proto_env_init(struct Library *SysBase, int *result);
void proto_env_exit(proto_env_handle_t *ph);

#endif
