#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "types.h"
#include "arch.h"

#include "channel.h"

#include "fwid.h"
#include "test.h"
#include "proto-testsuite.h"

UWORD test_size;
UWORD test_bias;
UWORD test_add_size;
UWORD test_sub_size;
UBYTE test_channel;

void tests_pamela_config(UWORD size, UWORD bias, UWORD add_size, UWORD sub_size,
                         UBYTE channel)
{
    test_size = size;
    test_bias = bias;
    test_add_size = add_size;
    test_sub_size = sub_size;
    test_channel = channel;
}

// ----- tests -----

int test_channel_init_exit(test_t *t, test_param_t *p)
{
    proto_env_handle_t *pe = (proto_env_handle_t *)p->user_data;

    /* init channel */
    channel_handle_t *ch = channel_init(test_channel, (struct Library *)SysBase, pe);
    if(ch == NULL) {
        p->error = "can't init";
        p->section = "init";
        return 1;
    }

    /* check status */
    UWORD status = channel_status(ch);
    if(status != CHANNEL_STATUS_UNKNOWN) {
        p->error = "invalid status: not CLOSED";
        p->section = "status";
        return 1;
    }

    /* exit channel */
    channel_exit(ch);

    return 0;
}

int test_channel_open_close(test_t *t, test_param_t *p)
{
    proto_env_handle_t *pe = (proto_env_handle_t *)p->user_data;

    /* init channel */
    channel_handle_t *ch = channel_init(test_channel, (struct Library *)SysBase, pe);
    if(ch == NULL) {
        p->error = "can't init";
        p->section = "init";
        return 1;
    }

    /* open channel */
    UWORD mtu_words = 512;
    int res = channel_open(ch, &mtu_words);
    if(res != CHANNEL_RET_OK) {
        p->error = channel_perror(res);
        p->section = "open";
        return 1;
    }

    /* check status */
    UWORD status = channel_status(ch);
    if(status != CHANNEL_STATUS_READY) {
        p->error = "invalid status: not ready";
        p->section = "after open";
        return 1;
    }

    /* close channel */
    res = channel_close(ch);
    if(res != CHANNEL_RET_OK) {
        p->error = channel_perror(res);
        p->section = "close";
        return 1;
    }

    /* check status */
    status = channel_status(ch);
    if(status != CHANNEL_STATUS_AVAILABLE) {
        p->error = "invalid status: not CLOSED";
        p->section = "after close";
        return 1;
    }

    /* exit channel */
    channel_exit(ch);

    return 0;
}
