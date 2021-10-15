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
#include "test-buffer.h"

test_buffer_param_t test_buf_param;
UBYTE test_channel;
UWORD test_mtu;

typedef int (*channel_test_func)(channel_handle_t *ch, test_t *t, test_param_t *p);

void tests_pamela_config(UWORD size, UWORD bias, UWORD add_size, UWORD sub_size,
                         UBYTE channel, UWORD mtu)
{
    test_buf_param.size = size;
    test_buf_param.bias = bias;
    test_buf_param.add_size = add_size;
    test_buf_param.sub_size = sub_size;
    test_channel = channel;
    test_mtu = mtu;
}

static int wait_status(channel_handle_t *ch, UWORD state)
{
    UWORD s;
    while(1) {
        s = channel_work(ch);
        if((s == state) || (s == CHANNEL_STATE_ERROR))
            break;
    }
    return s;
}

static int run_with_channel(test_t *t, test_param_t *p, channel_test_func test_func)
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
    UWORD mtu_words = 64;
    UWORD max_words = 512;
    int res = channel_open(ch, mtu_words, max_words);
    if(res != CHANNEL_RET_OK) {
        p->error = channel_perror(res);
        p->section = "open";
        return 1;
    }

    /* wait for open status */
    UWORD status = wait_status(ch, CHANNEL_STATE_IDLE);
    if(status != CHANNEL_STATE_IDLE) {
        p->error = "invalid state: not idle";
        p->section = "after open";
        return 1;
    }

    /* call user func */
    int result = test_func(ch, t, p);

    /* close channel */
    res = channel_close(ch);
    if(res != CHANNEL_RET_OK) {
        p->error = channel_perror(res);
        p->section = "close";
        return 1;
    }

    /* check status */
    status = wait_status(ch, CHANNEL_STATE_CLOSED);
    if(status != CHANNEL_STATE_CLOSED) {
        p->error = "invalid state: not CLOSED";
        p->section = "after close";
        return 1;
    }

    /* exit channel */
    channel_exit(ch);

    return result;
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
    UWORD state = channel_get_state(ch);
    if(state != CHANNEL_STATE_CLOSED) {
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
    UWORD mtu_words = 64;
    UWORD max_words = 512;
    int res = channel_open(ch, mtu_words, max_words);
    if(res != CHANNEL_RET_OK) {
        p->error = channel_perror(res);
        p->section = "open";
        return 1;
    }

    /* wait for open status */
    UWORD status = wait_status(ch, CHANNEL_STATE_IDLE);
    if(status != CHANNEL_STATE_IDLE) {
        p->error = "invalid state: not idle";
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
    status = wait_status(ch, CHANNEL_STATE_CLOSED);
    if(status != CHANNEL_STATE_CLOSED) {
        p->error = "invalid state: not CLOSED";
        p->section = "after close";
        return 1;
    }

    /* exit channel */
    channel_exit(ch);

    return 0;
}

int test_channel_read_func(channel_handle_t *ch, test_t *t, test_param_t *p)
{
    ULONG size = test_buffer_get_default_size(&test_buf_param);
    UBYTE *buf = test_buffer_alloc(size, p);
    if(buf == NULL) {
        return 1;
    }

    UWORD num_words = (UWORD)size >> 1;
    channel_message_t msg = {
        .data = buf,
        .num_words = num_words,
        .operation = CHANNEL_OP_READ
    };

    int res = channel_transfer(ch, &msg);
    if(res != CHANNEL_RET_OK) {
        p->error = channel_perror(res);
        p->section = "read";
        return 1;
    }

    /* wait for idle status */
    UWORD status = wait_status(ch, CHANNEL_STATE_IDLE);
    if(status != CHANNEL_STATE_IDLE) {
        p->error = "invalid state: not idle";
        p->section = "after read";
        return 1;
    }

    test_buffer_free(buf);
    return 0;
}

int test_channel_read(test_t *t, test_param_t *p)
{
    return run_with_channel(t, p, test_channel_read_func);
}

int test_channel_write_func(channel_handle_t *ch, test_t *t, test_param_t *p)
{
    ULONG size = test_buffer_get_default_size(&test_buf_param);
    UBYTE *buf = test_buffer_alloc(size, p);
    if(buf == NULL) {
        return 1;
    }

    UWORD num_words = (UWORD)size >> 1;
    channel_message_t msg = {
        .data = buf,
        .num_words = num_words,
        .operation = CHANNEL_OP_WRITE
    };

    int res = channel_transfer(ch, &msg);
    if(res != CHANNEL_RET_OK) {
        p->error = channel_perror(res);
        p->section = "read";
        return 1;
    }

    /* wait for idle status */
    UWORD status = wait_status(ch, CHANNEL_STATE_IDLE);
    if(status != CHANNEL_STATE_IDLE) {
        p->error = "invalid state: not idle";
        p->section = "after read";
        return 1;
    }

    test_buffer_free(buf);
    return 0;
}

int test_channel_write(test_t *t, test_param_t *p)
{
    return run_with_channel(t, p, test_channel_write_func);
}
