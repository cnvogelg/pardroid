#define __NOLIBBASE__
#include <proto/exec.h>

#include <exec/semaphores.h>

#ifdef CONFIG_DEBUG_ENGINE
#define KDEBUG
#endif

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "proto_shared.h"
#include "engine.h"
#include "worker.h"
#include "parbox.h"
#include "channel.h"

#define NUM_CHANNELS  PROTO_MAX_CHANNEL

struct engine_handle {
  worker_def_t      worker_def;
  parbox_handle_t   parbox;
  struct Library   *sys_base;
  struct SignalSemaphore sem;
  channel_t         *channels[NUM_CHANNELS];
  UWORD             result;
  UWORD             open_count;
};

#define SysBase eh->sys_base

static BOOL startup(void *user_data, ULONG *user_sig_mask)
{
  engine_handle_t *eh = (engine_handle_t *)user_data;

  /* setup parbox */
  parbox_handle_t *ph = &eh->parbox;
  int res = parbox_init(ph, eh->sys_base);
  if(res != PARBOX_OK) {
    eh->result = ENGINE_RET_INIT_FAILED;
    return FALSE;
  }

  /* setup signal-based timer */
  struct timer_handle *th = ph->timer;
  if(timer_sig_init(th) == -1) {
    parbox_exit(ph);
    eh->result = ENGINE_RET_INIT_FAILED | PARBOX_ERROR_TIMER;
    return FALSE;
  }

  /* store timer sigmask */
  *user_sig_mask = timer_sig_get_mask(th);

  eh->result = ENGINE_RET_OK;
  return TRUE;
}

static void shutdown(void *user_data)
{
  engine_handle_t *eh = (engine_handle_t *)user_data;
  parbox_handle_t *ph = &eh->parbox;

  /* cleanup signal-based timer */
  struct timer_handle *th = ph->timer;
  timer_sig_exit(th);

  /* cleanup parbox */
  parbox_exit(ph);
}

static BOOL do_write(engine_handle_t *eh, channel_t *c, request_t *r)
{
  proto_handle_t *ph = eh->parbox.proto;
  UWORD num_words = r->length >> 1;

  /* perform immediate write */
  int proto_res = proto_msg_write_single(ph, r->channel, r->data, num_words);
  if(proto_res != PROTO_RET_OK) {
    r->sub_error = (UBYTE)proto_res;
    r->error = PB_REQ_RET_PROTO_ERROR;
  } else {
    r->error = PB_REQ_RET_OK;
  }
  return TRUE;
}

static BOOL do_read(engine_handle_t *eh, channel_t *c, request_t *r)
{
  /* TODO */
  return TRUE;
}

static BOOL handle_msg(struct Message *msg, void *user_data)
{
  engine_handle_t *eh = (engine_handle_t *)user_data;
  request_t *r = (request_t *)msg;
  BOOL do_reply = TRUE;

  /* validate channel */
  UBYTE chn = r->channel;
  if(chn >= NUM_CHANNELS) {
    r->error = PB_REQ_RET_INVALID_CHANNEL;
    return TRUE;
  }

  /* lock channels array */
  ObtainSemaphore(&eh->sem);

  /* lookup channel and make sure its opened */
  channel_t *c = eh->channels[chn];
  if(c != NULL) {

    /* dispatch request by command */
    UBYTE cmd = r->cmd;
    D(("engine: got msg cmd=%ld\n", cmd));
    switch(cmd) {
      /* write a message immediately */
      case PB_REQ_MESSAGE_WRITE:
        do_reply = do_write(eh, c, r);
        break;
      /* read a message either immediately or postpone it */
      case PB_REQ_MESSAGE_READ:
        do_reply = do_read(eh, c, r);
        break;
      default:
        r->error = PB_REQ_RET_INVALID_CMD;
        break;
    }
  } else {
    /* invalid channel */
    r->error = PB_REQ_RET_CHANNEL_NOT_OPEN;
  }

  ReleaseSemaphore(&eh->sem);

  return do_reply;
}

static void handle_sig(ULONG sig_mask, void *user_data)
{
  engine_handle_t *eh = (engine_handle_t *)user_data;
  /* TODO */
}

#undef SysBase

engine_handle_t *engine_start(int *result, struct Library *SysBase, const char *port_name)
{
  engine_handle_t *eh;

  eh = AllocMem(sizeof(struct engine_handle), MEMF_CLEAR | MEMF_PUBLIC);
  if(eh == NULL) {
    return NULL;
  }

  /* setup worker task */
  worker_def_t *wd = &eh->worker_def;
  wd->task_name = "parbox_engine";
  wd->port_name = port_name;
  wd->user_data = eh;
  wd->startup = startup;
  wd->shutdown = shutdown;
  wd->handle_msg = handle_msg;
  wd->handle_sig = handle_sig;

  eh->sys_base = SysBase;

  InitSemaphore(&eh->sem);

  /* try to start worker */
  BOOL ok = worker_start(wd);
  *result = eh->result;
  if(!ok) {
    FreeMem(eh, sizeof(struct engine_handle));
    return NULL;
  } else {
    return eh;
  }
}

#define SysBase eh->sys_base

void engine_stop(engine_handle_t *eh)
{
  if(eh == NULL) {
    return;
  }

  /* close channels? */
  engine_close_all_channels(eh);

  /* stop worker */
  worker_def_t *wd = &eh->worker_def;
  worker_stop(wd);

  /* free handle */
  FreeMem(eh, sizeof(struct engine_handle));
}

int engine_open_channel(engine_handle_t *eh, UBYTE ch)
{
  int result = ENGINE_RET_OK;

  ObtainSemaphore(&eh->sem);
  if(eh->channels[ch] == NULL) {
    eh->channels[ch] = channel_create(ch);
    if(eh->channels[ch] == NULL) {
      result = ENGINE_RET_OUT_OF_MEMORY;
    } else {
      eh->open_count++;
    }
  } else {
    result = ENGINE_RET_OPEN_ERROR;
  }
  ReleaseSemaphore(&eh->sem);
  return result;
}

int engine_close_channel(engine_handle_t *eh, UBYTE ch)
{
  int result = ENGINE_RET_OK;

  ObtainSemaphore(&eh->sem);
  if(eh->channels[ch] != NULL) {
    channel_delete(eh->channels[ch]);
    eh->channels[ch] = NULL;
    eh->open_count--;
  } else {
    result = ENGINE_RET_CLOSE_ERROR;
  }
  ReleaseSemaphore(&eh->sem);
  return ENGINE_RET_OK;
}

int engine_close_all_channels(engine_handle_t *eh)
{
  int num_failed = 0;
  ObtainSemaphore(&eh->sem);
  if(eh->open_count > 0) {
    for(int i=0;i<NUM_CHANNELS;i++) {
      if(eh->channels[i] != NULL) {
        channel_delete(eh->channels[i]);
        eh->channels[i] = NULL;
        eh->open_count--;
      }
    }
  }
  ReleaseSemaphore(&eh->sem);
  return num_failed;
}

void engine_send_request(engine_handle_t *eh, request_t *r)
{
  PutMsg(eh->worker_def.port, (struct Message *)r);
}

const char *engine_perror(int res)
{
  switch(res & ENGINE_RET_MASK) {
    case ENGINE_RET_OK:
      return "OK";
    case ENGINE_RET_INIT_FAILED:
      return "init failed";
    case ENGINE_RET_OPEN_ERROR:
      return "open error";
    case ENGINE_RET_CLOSE_ERROR:
      return "open error";
    case ENGINE_RET_OUT_OF_MEMORY:
      return "out of memory";
    default:
      return "?";
  }
}
