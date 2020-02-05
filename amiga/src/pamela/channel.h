#ifndef CHANNEL_H
#define CHANNEL_H

#include "proto_env.h"

// channel ops return values
#define CHANNEL_RET_OK                  0
#define CHANNEL_RET_STAY                1
#define CHANNEL_RET_WRONG_STATE         2
#define CHANNEL_RET_WRONG_MSG           3
#define CHANNEL_RET_PROTO_ERROR         4
#define CHANNEL_RET_NO_AVAILABLE        5
#define CHANNEL_RET_OPEN_FAILED         6
#define CHANNEL_RET_CLOSE_FAILED        7
#define CHANNEL_RET_RESET_FAILED        8
#define CHANNEL_RET_DEVICE_ERROR        9
#define CHANNEL_RET_STATE_MISMATCH      10
#define CHANNEL_RET_CANCEL              11
#define CHANNEL_RET_MSG_SIZE_MISMATCH   12

// operations
#define CHANNEL_OP_READ                 0
#define CHANNEL_OP_WRITE                1

// channel state machine
#define CHANNEL_STATE_OPEN             0
#define CHANNEL_STATE_IDLE             1
#define CHANNEL_STATE_TRANSFER         2
#define CHANNEL_STATE_CANCEL           3
#define CHANNEL_STATE_CLOSE            4
#define CHANNEL_STATE_ERROR            5
#define CHANNEL_STATE_RESET            6
#define CHANNEL_STATE_POLL             7
#define CHANNEL_STATE_CLOSED           8

// opaque channel handle
struct channel_handle;
typedef struct channel_handle channel_handle_t;

// message
struct channel_message {
    proto_iov_t data;
    UWORD       num_words;
    UWORD       operation;
    ULONG       offset;
    // optional
    void        *user_data;
    // result
    UWORD       got_words;
    UWORD       result;
};
typedef struct channel_message channel_message_t;

// callback functions
typedef void (*channel_state_cb_t)(channel_handle_t *ch, UWORD state);
typedef void (*channel_transfer_cb_t)(channel_handle_t *ch, channel_message_t *msg);

// channel init/exit
extern channel_handle_t *channel_init(UBYTE channel_id, struct Library *SysBase,
                                      proto_env_handle_t *pe);
extern void channel_exit(channel_handle_t *ch);

// query info
extern const char *channel_perror(UWORD result);
extern UWORD channel_get_mtu(channel_handle_t *ch);
extern UWORD channel_get_max_words(channel_handle_t *ch);
extern UWORD channel_get_mode(channel_handle_t *ch);
extern UWORD channel_get_state(channel_handle_t *ch);
extern int channel_get_proto_error(channel_handle_t *ch);

// open and request an MTU (if != 0) or use default
extern int channel_open(channel_handle_t *ch, UWORD req_mtu, UWORD req_words);
extern int channel_close(channel_handle_t *ch);
extern void channel_reset(channel_handle_t *ch);
extern int channel_poll(channel_handle_t *ch);

// setup callbacks
extern void channel_set_state_cb(channel_handle_t *ch, channel_state_cb_t func);
extern void channel_set_transfer_cb(channel_handle_t *ch, channel_transfer_cb_t func);

// main worker call. returns resulting state
extern UWORD channel_work(channel_handle_t *ch);

// read/write message
extern int channel_transfer(channel_handle_t *ch, channel_message_t *msg);
extern int channel_cancel(channel_handle_t *ch, channel_message_t *msg);

#endif
