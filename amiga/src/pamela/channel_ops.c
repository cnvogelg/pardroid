#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "channel.h"
#include "channel_hnd.h"
#include "channel_ops.h"
#include "channel_shared.h"
#include "proto_env.h"
#include "proto.h"

#undef SysBase
#define SysBase ch->sys_base

static int set_channel_index(channel_handle_t *ch)
{
    int res = proto_wfunc_write(ch->proto, PROTO_WFUNC_WRITE_CHN_INDEX, ch->id);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_PROTO_ERROR;   
    }
    return CHANNEL_RET_OK;
}

static int get_dev_state(channel_handle_t *ch)
{
    UWORD status;

    int res = proto_wfunc_read(ch->proto, PROTO_WFUNC_READ_CHN_DEV_STATE, &status);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_PROTO_ERROR;
    }
    ch->dev_state = status;
    return CHANNEL_RET_OK;
}

int channel_op_open(channel_handle_t *ch)
{
    /* select my channel index on device */
    int res = set_channel_index(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* check/update state on device */
    res = get_dev_state(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* if channel is not closed then fail */
    if(ch->dev_state != CHANNEL_DEV_STATE_AVAILABLE) {
        return CHANNEL_RET_OPEN_FAILED;
    }

    /* try to set mtu (if != 0) */
    UWORD mtu = ch->req_mtu;
    if(mtu != 0) {
        res = proto_wfunc_write(ch->proto, PROTO_WFUNC_WRITE_CHN_MTU, mtu);
        if(res != PROTO_RET_OK) {
            ch->proto_error = res;
            return CHANNEL_RET_PROTO_ERROR;
        }
    }

    /* get final MTU from device */
    res = proto_wfunc_read(ch->proto, PROTO_WFUNC_READ_CHN_MTU, &mtu);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_PROTO_ERROR;
    }
    ch->mtu = mtu;

    /* try to set max words (if != 0) */
    UWORD max_words = ch->req_words;
    if(max_words != 0) {
        res = proto_wfunc_write(ch->proto, PROTO_WFUNC_WRITE_CHN_MAX_WORDS, max_words);
        if(res != PROTO_RET_OK) {
            ch->proto_error = res;
            return CHANNEL_RET_PROTO_ERROR;
        }
    }

    /* get final MTU from device */
    res = proto_wfunc_read(ch->proto, PROTO_WFUNC_READ_CHN_MAX_WORDS, &max_words);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_PROTO_ERROR;
    }
    ch->max_words = max_words;
   
    /* get mode from device */
    UWORD mode = 0;
    res = proto_wfunc_read(ch->proto, PROTO_WFUNC_READ_CHN_MODE, &mode);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_PROTO_ERROR;
    }
    ch->mode = mode;

    /* call open on device */
    res = proto_action(ch->proto, PROTO_ACTION_CHN_OPEN);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_PROTO_ERROR;
    }

    /* finally check status of device channel */
    res = get_dev_state(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* status should be 'ready' */
    if(ch->dev_state == CHANNEL_DEV_STATE_READY) {
        return CHANNEL_RET_OK;
    } else {
        return CHANNEL_RET_OPEN_FAILED;
    }
}

int channel_op_close(channel_handle_t *ch)
{
    /* select my channel index on device */
    int res = set_channel_index(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* update status of device channel */
    res = get_dev_state(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* status has to be ready */
    if(ch->dev_state != CHANNEL_DEV_STATE_READY) {
        return CHANNEL_RET_CLOSE_FAILED;
    }

    /* call close */
    res = proto_action(ch->proto, PROTO_ACTION_CHN_CLOSE);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_PROTO_ERROR;
    }

    /* update status of device channel */
    res = get_dev_state(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* status should be 'available' */
    if(ch->dev_state == CHANNEL_DEV_STATE_AVAILABLE) {
        return CHANNEL_RET_OK;
    } else {
        return CHANNEL_RET_CLOSE_FAILED;
    }
}

int channel_op_reset(channel_handle_t *ch)
{
    /* select my channel index on device */
    int res = set_channel_index(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* directly trigger reset */
    res = proto_action(ch->proto, PROTO_ACTION_CHN_RESET);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_PROTO_ERROR;
    }

    /* update status */
    res = get_dev_state(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* status should be 'available' */
    if(ch->dev_state == CHANNEL_DEV_STATE_AVAILABLE) {
        return CHANNEL_RET_OK;
    } else {
        return CHANNEL_RET_RESET_FAILED;
    }
}

static int advance_iov(channel_handle_t *ch, UWORD size)
{
    proto_iov_t *iov = &ch->tr_iov;
    while(size > 0) {
        // iov is too small -> move on
        if(iov->num_words < size) {
            // no next one???
            if(iov->next == NULL) {
                return CHANNEL_RET_MSG_SIZE_MISMATCH;
            }
            size -= iov->num_words;
            // advance to next one
            *iov = *iov->next;
        }
        else {
            // adjust current iov
            iov->data += size<<1;
            iov->num_words -= size;
            size = 0;
        }
    }
    return CHANNEL_RET_OK;
}

static int read_op(channel_handle_t *ch)
{
    channel_message_t *msg = ch->tr_msg;

    // begin of read
    if(ch->tr_words == 0) {
        // send offset
        if(ch->mode & CHANNEL_MODE_OFFSET) {
            int res = proto_chn_set_offset(ch->proto, ch->id, msg->offset);
            if(res != PROTO_RET_OK) {
                ch->proto_error = res;
                return CHANNEL_RET_PROTO_ERROR;
            } 
        }

        // get message size
        int res = proto_chn_get_rx_size(ch->proto, ch->id, &ch->tr_words);
        if(res != PROTO_RET_OK) {
            ch->proto_error = res;
            return CHANNEL_RET_PROTO_ERROR;
        }

        // check size of message buffer
        if(ch->tr_words > msg->num_words) {
            return CHANNEL_RET_MSG_SIZE_MISMATCH;
        }

        msg->got_words = ch->tr_words;

        // empty message? -> done
        if(ch->tr_words == 0) {
            return CHANNEL_RET_OK;
        }
    }

    // limit transfer size to MTU
    UWORD size = ch->tr_words;
    if(size > ch->mtu) {
        size = ch->mtu;
    }

    // transfer a chunk
    int res = proto_chn_msg_readv(ch->proto, ch->id, &ch->tr_iov, size);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_PROTO_ERROR;
    }

    // update transfer state
    res = advance_iov(ch, size);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    ch->tr_words -= size;
    if(ch->tr_words == 0) {
        return CHANNEL_RET_OK;
    } else {
        return CHANNEL_RET_STAY;
    }
}

static int write_op(channel_handle_t *ch)
{
    channel_message_t *msg = ch->tr_msg;

    // begin of write
    if(ch->tr_words == 0) {
        // send offset
        if(ch->mode & CHANNEL_MODE_OFFSET) {
            int res = proto_chn_set_offset(ch->proto, ch->id, msg->offset);
            if(res != PROTO_RET_OK) {
                ch->proto_error = res;
                return CHANNEL_RET_PROTO_ERROR;
            } 
        }

        // send size of message
        int res = proto_chn_set_tx_size(ch->proto, ch->id, msg->num_words);
        if(res != PROTO_RET_OK) {
            ch->proto_error = res;
            return CHANNEL_RET_PROTO_ERROR;
        }
        ch->tr_words = msg->num_words;

        msg->got_words = ch->tr_words;

        // empty message? -> done
        if(ch->tr_words == 0) {
            return CHANNEL_RET_OK;
        }
    }

    // limit transfer size to MTU
    UWORD size = ch->tr_words;
    if(size > ch->mtu) {
        size = ch->mtu;
    }

    // transfer a chunk
    int res = proto_chn_msg_writev(ch->proto, ch->id, &ch->tr_iov, size);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_PROTO_ERROR;
    }

    // update transfer state
    res = advance_iov(ch, size);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    ch->tr_words -= size;
    if(ch->tr_words == 0) {
        return CHANNEL_RET_OK;
    } else {
        return CHANNEL_RET_STAY;
    }
}

int channel_op_transfer(channel_handle_t *ch)
{
    channel_message_t *msg = ch->tr_msg;
    int res;

    // dispatch to read/write
    if(msg->operation == CHANNEL_OP_READ) {
        res = read_op(ch);
    }
    else {
        res = write_op(ch);
    } 

    // transform device busy and stay in transfer
    if((res == CHANNEL_RET_PROTO_ERROR) && (ch->proto_error = PROTO_RET_DEVICE_BUSY)) {
        res = CHANNEL_RET_STAY;
    }

    // if we are ready or got an error then report via cb
    if(res != CHANNEL_RET_STAY) {
        msg->result = res;
        msg->got_words -= ch->tr_words;

        ch->tr_words = 0;
        ch->tr_msg = NULL;

        if(ch->transfer_cb != NULL) {
            ch->transfer_cb(ch, msg);
        }   
    }
    return res;
}

int channel_op_cancel(channel_handle_t *ch)
{
    channel_message_t *msg = ch->tr_msg;
    
    // cancel state
    msg->result = CHANNEL_RET_CANCEL;
    msg->got_words -= ch->tr_words;

    ch->tr_words = 0;
    ch->tr_msg = NULL;

    // report msg as cancelled
    if(ch->transfer_cb != NULL) {
        ch->transfer_cb(ch, msg);
    }

    return CHANNEL_RET_OK;
}

int channel_op_poll(channel_handle_t *ch)
{
    /* update status */
    int res = get_dev_state(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* check device state */
    switch(ch->dev_state) {
        case CHANNEL_DEV_STATE_UNAVAILABLE:
        case CHANNEL_DEV_STATE_AVAILABLE:
            return CHANNEL_RET_STATE_MISMATCH;
        case CHANNEL_DEV_STATE_READY:
            return CHANNEL_RET_OK;
        case CHANNEL_DEV_STATE_ERROR:
        default:
            return CHANNEL_RET_DEVICE_ERROR;
    }
}
