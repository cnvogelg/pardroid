#ifndef PAMELA_H
#define PAMELA_H

#include "pario.h"
#include "timer.h"
#include "proto.h"
#include "proto_shared.h"
#include "reg.h"

/* pamela init error codes */
#define PAMELA_OK               0
#define PAMELA_ERROR_PARIO      1
#define PAMELA_ERROR_TIMER      2
#define PAMELA_ERROR_PROTO      3
#define PAMELA_ERROR_NO_SIGNAL  4
#define PAMELA_ERROR_ACK_IRQ    5
#define PAMELA_ERROR_TIMER_SIG  6
#define PAMELA_ERROR_RESET      7
#define PAMELA_ERROR_BOOTLOADER 8

/* init flags */
#define PAMELA_INIT_NORMAL      0
#define PAMELA_INIT_BOOT        1

struct pamela_handle;
typedef struct pamela_handle pamela_handle_t;

pamela_handle_t *pamela_init(struct Library *SysBase, int *res, int flags);
void pamela_exit(pamela_handle_t *ph);

proto_handle_t *pamela_get_proto(pamela_handle_t *ph);
timer_handle_t *pamela_get_timer(pamela_handle_t *ph);

int pamela_init_events(pamela_handle_t *ph);
void pamela_exit_events(pamela_handle_t *ph);
ULONG pamela_wait_event(pamela_handle_t *ph,
                        ULONG timeout_s, ULONG timeout_us, ULONG extra_sigmask);
ULONG pamela_get_event_sigmask(pamela_handle_t *ph);
ULONG pamela_get_timer_sigmask(pamela_handle_t *ph);
UWORD pamela_get_num_events(pamela_handle_t *ph);
UWORD pamela_get_num_event_signals(pamela_handle_t *ph);

const char *pamela_perror(int res);

#endif /* PAMELA_H */
