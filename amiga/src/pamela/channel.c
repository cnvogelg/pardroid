#define __NOLIBBASE__
#include <proto/exec.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "channel.h"
#include "proto_env.h"
#include "proto.h"

struct channel_handle
{
    proto_env_handle_t *proto_env;
    proto_handle_t     *proto;
    struct Library     *sys_base;
    int                 proto_error;
    UWORD               status;
    UWORD               mode;
    UBYTE               id;
};

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
    ch->status = CHANNEL_STATUS_UNKNOWN;
    ch->mode = 0;

    return ch;
}

#undef SysBase
#define SysBase ch->sys_base

void channel_exit(channel_handle_t *ch)
{
    FreeMem(ch, sizeof(channel_handle_t));
}

UWORD channel_status(channel_handle_t *ch)
{
    return ch->status;
}

UWORD channel_mode(channel_handle_t *ch)
{
    return ch->mode;
}

int channel_proto_error(channel_handle_t *ch)
{
    return ch->proto_error;
}

const char *channel_perror(UWORD status)
{
    switch(status) {
        case CHANNEL_RET_OK:
            return "OK";
        case CHANNEL_RET_BUSY:
            return "busy";
        case CHANNEL_RET_PARTIAL:
            return "partial";
        case CHANNEL_RET_ERROR_SET_INDEX:
            return "can't set channel index";
        case CHANNEL_RET_ERROR_SET_MTU:
            return "can't set MTU";
        case CHANNEL_RET_ERROR_GET_MTU:
            return "can't get MTU";
        case CHANNEL_RET_ERROR_GET_MODE:
            return "can't get mode";
        case CHANNEL_RET_ERROR_NOT_AVAIL:
            return "not available";
        case CHANNEL_RET_ERROR_CALL_OPEN:
            return "can't call open";
        case CHANNEL_RET_ERROR_OPEN_FAILED:
            return "open failed on device";
        case CHANNEL_RET_ERROR_NOT_READY:
            return "not ready";
        case CHANNEL_RET_ERROR_CALL_CLOSE:
            return "can't call close";
        case CHANNEL_RET_ERROR_CLOSE_FAILED:
            return "close failed on device";
        default:
            return "???";
    }
}

static int select_index(channel_handle_t *ch)
{
    int res = proto_wfunc_write(ch->proto, PROTO_WFUNC_WRITE_CHN_INDEX, ch->id);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_ERROR_SET_INDEX;   
    }
    return CHANNEL_RET_OK;
}

static int update_status(channel_handle_t *ch)
{
    UWORD status;

    int res = proto_wfunc_read(ch->proto, PROTO_WFUNC_READ_CHN_STATUS, &status);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_ERROR_GET_STATUS;
    }
    ch->status = status;
    return CHANNEL_RET_OK;
}

int channel_open(channel_handle_t *ch, UWORD *mtu_words)
{
    /* select my channel index on device */
    int res = select_index(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* check/update state on device */
    res = update_status(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* if channel is not closed then fail */
    if(ch->status != CHANNEL_STATUS_AVAILABLE) {
        return CHANNEL_RET_ERROR_NOT_AVAIL;
    }

    /* try to set mtu (if != 0) */
    UWORD mtu = *mtu_words;
    if(mtu != 0) {
        res = proto_wfunc_write(ch->proto, PROTO_WFUNC_WRITE_CHN_MTU, mtu);
        if(res != PROTO_RET_OK) {
            ch->proto_error = res;
            return CHANNEL_RET_ERROR_SET_MTU;
        }
    }

    /* get final MTU from device */
    res = proto_wfunc_read(ch->proto, PROTO_WFUNC_READ_CHN_MTU, &mtu);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_ERROR_GET_MTU;
    }

    /* get mode from device */
    UWORD mode = 0;
    res = proto_wfunc_read(ch->proto, PROTO_WFUNC_READ_CHN_MODE, &mode);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_ERROR_GET_MODE;
    }
    ch->mode = mode;

    /* call open on device */
    res = proto_action(ch->proto, PROTO_ACTION_CHN_OPEN);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_ERROR_CALL_OPEN;
    }

    /* finally check status of device channel */
    res = update_status(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* status should be 'ready' */
    if(ch->status == CHANNEL_STATUS_READY) {
        return CHANNEL_RET_OK;
    } else {
        return CHANNEL_RET_ERROR_OPEN_FAILED;
    }
}

int channel_close(channel_handle_t *ch)
{
    /* select my channel index on device */
    int res = select_index(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* update status of device channel */
    res = update_status(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* status has to be ready */
    if(ch->status != CHANNEL_STATUS_READY) {
        return CHANNEL_RET_ERROR_NOT_READY;
    }

    /* call close */
    res = proto_action(ch->proto, PROTO_ACTION_CHN_CLOSE);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_ERROR_CALL_CLOSE;
    }

    /* update status of device channel */
    res = update_status(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* status should be 'available' */
    if(ch->status == CHANNEL_STATUS_AVAILABLE) {
        return CHANNEL_RET_OK;
    } else {
        return CHANNEL_RET_ERROR_CLOSE_FAILED;
    }
}

int channel_reset(channel_handle_t *ch)
{
    /* select my channel index on device */
    int res = select_index(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* directly trigger reset */
    res = proto_action(ch->proto, PROTO_ACTION_CHN_RESET);
    if(res != PROTO_RET_OK) {
        ch->proto_error = res;
        return CHANNEL_RET_ERROR_CALL_RESET;
    }

    /* update status */
    res = update_status(ch);
    if(res != CHANNEL_RET_OK) {
        return res;
    }

    /* status should be 'available' */
   if(ch->status == CHANNEL_STATUS_AVAILABLE) {
        return CHANNEL_RET_OK;
    } else {
        return CHANNEL_RET_ERROR_CLOSE_FAILED;
    }
}
