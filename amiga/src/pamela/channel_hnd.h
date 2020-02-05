#ifndef CHANNEL_HND_H
#define CHANNEL_HND_H

struct channel_handle
{
    proto_env_handle_t *proto_env;
    proto_handle_t     *proto;
    struct Library     *sys_base;

    channel_state_cb_t    state_cb;
    channel_transfer_cb_t transfer_cb;

    channel_message_t  *tr_msg;

    UWORD               state;
    UWORD               mode;
    UWORD               mtu;
    UWORD               dev_state;

    UWORD               max_words;
    UWORD               req_mtu;
    UWORD               req_words;

    UWORD               tr_words;
    proto_iov_t         tr_iov;

    int                 error_code;
    int                 proto_error;
    UBYTE               id;
};

#endif
