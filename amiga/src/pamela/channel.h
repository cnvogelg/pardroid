#ifndef CHANNEL_H
#define CHANNEL_H

#include "proto_env.h"

// channel modes (mask)
#define CHANNEL_MODE_RX_VAR_SIZE        0x01
#define CHANNEL_MODE_TX_VAR_SIZE        0x02
#define CHANNEL_MODE_VAR_SIZE           0x03
#define CHANNEL_MODE_RX_OFFSET          0x04
#define CHANNEL_MODE_TX_OFFSET          0x08
#define CHANNEL_MODE_OFFSET             0x0c

// channel ops return values
#define CHANNEL_RET_OK                  0
#define CHANNEL_RET_BUSY                1  // device is busy
#define CHANNEL_RET_PARTIAL             2  // read/write is not finished yet
// errors
#define CHANNEL_RET_ERROR_SET_INDEX     3
#define CHANNEL_RET_ERROR_GET_STATUS    4
#define CHANNEL_RET_ERROR_SET_MTU       5
#define CHANNEL_RET_ERROR_GET_MTU       6
#define CHANNEL_RET_ERROR_GET_MODE      7
#define CHANNEL_RET_ERROR_NOT_AVAIL     8
#define CHANNEL_RET_ERROR_CALL_OPEN     9
#define CHANNEL_RET_ERROR_OPEN_FAILED   10
#define CHANNEL_RET_ERROR_NOT_READY     11
#define CHANNEL_RET_ERROR_CALL_CLOSE    12
#define CHANNEL_RET_ERROR_CLOSE_FAILED  13
#define CHANNEL_RET_ERROR_CALL_RESET    14
#define CHANNEL_RET_ERROR_RESET_FAILED  15

// channel status of device
#define CHANNEL_STATUS_UNKNOWN          0
#define CHANNEL_STATUS_AVAILABLE        1
#define CHANNEL_STATUS_READY            2
#define CHANNEL_STATUS_READ_PENDING     3
#define CHANNEL_STATUS_IN_READ          4
#define CHANNEL_STATUS_IN_WRITE         5
// error states
#define CHANNEL_STATUS_ERROR            0x10
#define CHANNEL_STATUS_OPEN_ERROR       0x11
#define CHANNEL_STATUS_CLOSE_ERROR      0x12
#define CHANNEL_STATUS_WRITE_ERROR      0x13
#define CHANNEL_STATUS_READ_ERROR       0x14

// opaque channel handle
struct channel_handle;
typedef struct channel_handle channel_handle_t;

// channel ops
extern channel_handle_t *channel_init(UBYTE channel_id, struct Library *SysBase,
                                      proto_env_handle_t *pe);
extern void channel_exit(channel_handle_t *ch);

extern const char *channel_perror(UWORD status);

// open and request an MTU (if != 0) and get effective MTU back
extern int channel_open(channel_handle_t *ch, UWORD *mtu_words);
extern int channel_close(channel_handle_t *ch);
extern int channel_reset(channel_handle_t *ch);

// query info
extern UWORD channel_mode(channel_handle_t *ch);
extern UWORD channel_status(channel_handle_t *ch);
extern int channel_proto_error(channel_handle_t *ch);

// simple read/write (spread across multiple mtus if necessary)
extern int channel_write(channel_handle_t *ch, proto_iov_t *msgiov, ULONG offset);
extern int channel_read(channel_handle_t *ch, proto_iov_t *msgiov, ULONG offset, UWORD *ret_words);

// partial read/write (max one mtu per call)
extern int channel_write_begin(channel_handle_t *ch, proto_iov_t *msgiov, ULONG offset);
extern int channel_write_next(channel_handle_t *ch);
extern int channel_write_cancel(channel_handle_t *ch);
extern int channel_write_end(channel_handle_t *ch);

extern int channel_read_begin(channel_handle_t *ch, proto_iov_t *msgiov, ULONG offset);
extern int channel_read_next(channel_handle_t *ch);
extern int channel_read_cancel(channel_handle_t *ch);
extern int channel_read_end(channel_handle_t *ch, UWORD *ret_words);

#endif
