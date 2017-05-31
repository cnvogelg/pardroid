#include <proto/exec.h>
#include <proto/alib.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "channel.h"

channel_t *channel_create(UBYTE id)
{
  channel_t *chn = (channel_t *)AllocMem(sizeof(channel_t), MEMF_CLEAR | MEMF_PUBLIC);
  if(chn != NULL) {
    chn->id = id;
    NewList(&chn->read_requests);
    InitSemaphore(&chn->sem);
  }
  return chn;
}

void channel_delete(channel_t *chn)
{
  if(chn == NULL) {
    return;
  }
  FreeMem(chn, sizeof(channel_t));
}

void channel_add_read_request(channel_t *chn, request_t *req)
{
  ObtainSemaphore(&chn->sem);
  AddTail(&chn->read_requests, (struct Node *)req);
  ReleaseSemaphore(&chn->sem);
}

request_t *channel_get_next_read_request(channel_t *chn)
{
  request_t *result = NULL;
  ObtainSemaphore(&chn->sem);
  result = (request_t *)RemHead(&chn->read_requests);
  ReleaseSemaphore(&chn->sem);
  return result;
}