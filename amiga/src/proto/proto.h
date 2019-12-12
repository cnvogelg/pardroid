#ifndef PROTO_H
#define PROTO_H

#include "pario.h"
#include "timer.h"
#include "proto_shared.h"
#include "proto_iov.h"

#define PROTO_RET_OK                0
#define PROTO_RET_RAK_INVALID       1
#define PROTO_RET_TIMEOUT           2
#define PROTO_RET_SLAVE_ERROR       3
#define PROTO_RET_MSG_TOO_LARGE     4
#define PROTO_RET_DEVICE_BUSY       5
#define PROTO_RET_INVALID_CHANNEL   15
#define PROTO_RET_INVALID_FUNCTION  14
#define PROTO_RET_INVALID_ACTION    13
#define PROTO_RET_INVALID_MTU       12

struct proto_handle;
typedef struct proto_handle proto_handle_t;

extern proto_handle_t *proto_init(struct pario_port *port, struct timer_handle *th, struct Library *SysBase);
extern void proto_exit(proto_handle_t *ph);

extern int proto_ping(proto_handle_t *ph);
extern int proto_reset(proto_handle_t *ph);
extern int proto_bootloader(proto_handle_t *ph);
extern int proto_knok(proto_handle_t *ph);

extern int proto_action(proto_handle_t *ph, UBYTE num);
extern int proto_action_no_busy(proto_handle_t *ph, UBYTE num);
extern int proto_action_bench(proto_handle_t *ph, UBYTE num, ULONG deltas[2]);

extern int proto_wfunc_read(proto_handle_t *ph, UBYTE num, UWORD *data);
extern int proto_wfunc_write(proto_handle_t *ph, UBYTE num, UWORD data);

extern int proto_lfunc_read(proto_handle_t *ph, UBYTE num, ULONG *data);
extern int proto_lfunc_write(proto_handle_t *ph, UBYTE num, ULONG data);

// channel commands
extern int proto_chn_msg_writev(proto_handle_t *ph, UBYTE chn, proto_iov_t *msgiov);
extern int proto_chn_msg_readv(proto_handle_t *ph, UBYTE chn, proto_iov_t *msgiov);
extern int proto_chn_msg_write(proto_handle_t *ph, UBYTE chn, UBYTE *buf, UWORD num_words);
extern int proto_chn_msg_read(proto_handle_t *ph, UBYTE chn, UBYTE *buf, UWORD num_words);

// extended commands
extern int proto_chn_get_rx_size(proto_handle_t *ph, UBYTE chn, UWORD *rx_size);
extern int proto_chn_set_rx_size(proto_handle_t *ph, UBYTE chn, UWORD rx_size);
extern int proto_chn_set_tx_size(proto_handle_t *ph, UBYTE chn, UWORD tx_size);
extern int proto_chn_set_rx_offset(proto_handle_t *ph, UBYTE chn, ULONG rx_offset);
extern int proto_chn_set_tx_offset(proto_handle_t *ph, UBYTE chn, ULONG tx_offset);
extern int proto_chn_request_rx(proto_handle_t *ph, UBYTE chn);
extern int proto_chn_cancel_rx(proto_handle_t *ph, UBYTE chn);
extern int proto_chn_cancel_tx(proto_handle_t *ph, UBYTE chn);

extern const char *proto_perror(int res);

#endif
