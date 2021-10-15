#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "channel.h"
#include "channel_hnd.h"
#include "channel_ops.h"
#include "proto_env.h"
#include "proto.h"

channel_handle_t *channel_init(UBYTE channel_id, struct Library *SysBase,
                               proto_env_handle_t *proto_env)
{
    channel_handle_t *ch = AllocMem(sizeof(channel_handle_t), MEMF_CLEAR | MEMF_PUBLIC);
    if(ch == NULL) {
        return NULL;
    }

    ch->sys_base = SysBase;
    ch->proto_env = proto_env;
    ch->proto = proto_env_get_proto(proto_env);
    ch->id = channel_id;
    ch->state = CHANNEL_STATE_CLOSED;

    return ch;
}

#undef SysBase
#define SysBase ch->sys_base

void channel_exit(channel_handle_t *ch)
{
    FreeMem(ch, sizeof(channel_handle_t));
}

// query info

const char *channel_perror(UWORD result)
{
    switch(result) {
        case CHANNEL_RET_OK:
            return "OK";
        case CHANNEL_RET_STAY:
            return "stay";
        case CHANNEL_RET_WRONG_STATE:
            return "wrong state";
        case CHANNEL_RET_WRONG_MSG:
            return "wrong message";
        case CHANNEL_RET_PROTO_ERROR:
            return "protocol error";
        default:
            return "???";
    }
}

UWORD channel_get_mtu(channel_handle_t *ch)
{
    return ch->mtu;
}

UWORD channel_get_max_words(channel_handle_t *ch)
{
    return ch->max_words;
}

UWORD channel_get_mode(channel_handle_t *ch)
{
    return ch->mode;
}

UWORD channel_get_state(channel_handle_t *ch)
{
    return ch->state;
}

int channel_get_error_code(channel_handle_t *ch)
{
    return ch->error_code;
}

int channel_get_proto_error(channel_handle_t *ch)
{
    return ch->proto_error;
}

// state change

int channel_open(channel_handle_t *ch, UWORD req_mtu, UWORD req_words)
{
    if(ch->state != CHANNEL_STATE_CLOSED) {
        return CHANNEL_RET_WRONG_STATE;
    }

    ch->state = CHANNEL_STATE_OPEN;
    ch->req_mtu = req_mtu;
    ch->req_words = req_words;
    return CHANNEL_RET_OK;
}

int channel_close(channel_handle_t *ch)
{
    if(ch->state == CHANNEL_STATE_IDLE) {
        ch->state = CHANNEL_STATE_CLOSE;
        return CHANNEL_RET_OK;
    }

    if(ch->state == CHANNEL_STATE_OPEN) {
        ch->state = CHANNEL_STATE_IDLE;
        return CHANNEL_RET_OK;
    }

    return CHANNEL_RET_WRONG_STATE;
}

void channel_reset(channel_handle_t *ch)
{
    ch->state = CHANNEL_STATE_RESET;
}

int channel_poll(channel_handle_t *ch)
{
    if(ch->state != CHANNEL_STATE_IDLE) {
        return CHANNEL_RET_WRONG_STATE;
    }

    ch->state = CHANNEL_STATE_POLL;
    return CHANNEL_RET_OK;
}

// setup callbacks
void channel_set_state_cb(channel_handle_t *ch, channel_state_cb_t func)
{
    ch->state_cb = func;
}

void channel_set_transfer_cb(channel_handle_t *ch, channel_transfer_cb_t func)
{
    ch->transfer_cb = func;
}

// main worker call. returns current state
UWORD channel_work(channel_handle_t *ch)
{
    UWORD state = ch->state;

    // nothing to be done. exit early.
    if((state == CHANNEL_STATE_CLOSED) ||
       (state == CHANNEL_STATE_IDLE)) {
        return state;
    }

    // work on state
    int result = CHANNEL_RET_OK;
    switch(state) {
        case CHANNEL_STATE_OPEN:
            result = channel_op_open(ch);
            state = CHANNEL_STATE_IDLE;
            break;
        case CHANNEL_STATE_CLOSE:
            result = channel_op_close(ch);
            state = CHANNEL_STATE_CLOSED;
            break;
        case CHANNEL_STATE_TRANSFER:
            result = channel_op_transfer(ch);
            state = CHANNEL_STATE_IDLE;
            break;
        case CHANNEL_STATE_CANCEL:
            result = channel_op_cancel(ch);
            state = CHANNEL_STATE_IDLE;
            break;
        case CHANNEL_STATE_RESET:
            result = channel_op_reset(ch);
            state = CHANNEL_STATE_CLOSED;
            break;
        case CHANNEL_STATE_POLL:
            result = channel_op_poll(ch);
            state = CHANNEL_STATE_IDLE;
            break;
    }

    // state action was successful so update to new state
    if(result == CHANNEL_RET_OK) {
        ch->state = state;
        // trigger state callback
        if(ch->state_cb != NULL) {
            ch->state_cb(ch, state);
        }
    }
    // stay in state
    else if(result == CHANNEL_RET_STAY) {
        return ch->state;
    }
    // check for proto error
    // if device is busy we stay in state and try again later
    else if((result == CHANNEL_RET_PROTO_ERROR) &&
            (ch->proto_error == PROTO_RET_DEVICE_BUSY)) {
        return ch->state;
    }
    // other proto errors are fatal
    else  {
        ch->state = CHANNEL_STATE_ERROR;
        ch->error_code = result;
    }

    return ch->state;
}

// read/write message
int channel_transfer(channel_handle_t *ch, channel_message_t *msg)
{
    // assume not null message
    if(msg == NULL) {
        return CHANNEL_RET_WRONG_MSG;
    }
    if(ch->state != CHANNEL_STATE_IDLE) {
        return CHANNEL_RET_WRONG_STATE;
    }

    ch->state = CHANNEL_STATE_TRANSFER;
    ch->tr_msg = msg;
    ch->tr_words = 0;
    ch->tr_buf = msg->data;
    return CHANNEL_RET_OK;
}

int channel_cancel(channel_handle_t *ch, channel_message_t *msg)
{
    // assume not null message
    if(msg == NULL) {
        return CHANNEL_RET_WRONG_MSG;
    }
    if(ch->state != CHANNEL_STATE_TRANSFER) {
        return CHANNEL_RET_WRONG_STATE;
    }
    // make sure we cancel the message being transferred
    if(ch->tr_msg != msg) {
        return CHANNEL_RET_WRONG_MSG;
    }

    ch->state = CHANNEL_STATE_CANCEL;
    return CHANNEL_RET_OK;
}
