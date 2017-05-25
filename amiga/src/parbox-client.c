#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "engine.h"
#include "request.h"

static const char *TEMPLATE =
   "N=NumBlks/K/N"
   "S=BlkSize/K/N"
   "C=Channel/K/N"
   "O=Operation/K"
   "P=PortName/K";
typedef struct {
  ULONG *num_blks;
  ULONG *blk_size;
  ULONG *channel;
  char *operation;
  char *port_name;
} params_t;
typedef struct {
  ULONG  num_blks;
  ULONG  blk_size;
  UBYTE  channel;
  UBYTE  operation;
  char  *port_name;
} options_t;
static params_t params;

typedef struct {
  struct MsgPort *engine_port;
  UBYTE *buf;
  struct MsgPort *my_port;
  request_t *req;
} data_t;

#define OPER_WRITE  'w'
#define OPER_READ   'r'
#define OPER_PING   'p'

int run_write(options_t *opts, data_t *data)
{
  request_t *r = data->req;

  /* fill in request */
  r->cmd = PB_REQ_MESSAGE_WRITE;
  r->channel = 0;
  r->data = "hallo!";
  r->length = 6;

  PutStr("send write request\n");
  PutMsg(data->engine_port, (struct Message *)r);
  PutStr("waiting for reply...\n");
  WaitPort(data->my_port);
  request_t *r_get = (request_t *)GetMsg(data->my_port);
  if(r_get == r) {
    Printf("returned write: error=%ld\n", r->error);
  } else {
    Printf("invalid request returned=%ld\n", r_get);
  }

  return RETURN_OK;
}

int run_operation(options_t *opts, data_t *data)
{
  switch(opts->operation) {
    case OPER_WRITE:
      return run_write(opts, data);
    default:
      return RETURN_ERROR;
  }
}

int run_client(options_t *opts)
{
  Printf("num_blks=%ld, blk_size=%ld, channel=%ld, operation=%lc, port='%s'\n",
    opts->num_blks, opts->blk_size, opts->channel, opts->operation, opts->port_name);

  /* first try to find engine's public port */
  Forbid();
  struct MsgPort *port = FindPort(opts->port_name);
  Permit();
  if(port == NULL) {
    PutStr("Parbox engine's port was not found!\nDid you run 'parbox-engine' first?\n");
    return RETURN_ERROR;
  }

  int res = RETURN_OK;

  /* allocate buffer */
  UBYTE *buf = AllocVec(opts->blk_size, MEMF_CLEAR | MEMF_PUBLIC);
  if(buf != NULL) {

    /* allocate reply port */
    struct MsgPort *my_port = CreateMsgPort();
    if(my_port != NULL) {

      /* allocate request */
      request_t *req = request_create(my_port);
      if(req != NULL) {

        data_t data;
        data.engine_port = port;
        data.buf = buf;
        data.my_port = my_port;
        data.req = req;
        res = run_operation(opts, &data);

        request_delete(req);
      } else {
        PutStr("Can't create request!\n");
        res = RETURN_ERROR;
      }
      DeleteMsgPort(my_port);
    } else {
      PutStr("Can't allocate port!\n");
      res = RETURN_ERROR;
    }
    FreeVec(buf);
  } else {
    PutStr("No memory for buffer!\n");
    res = RETURN_ERROR;
  }

  return res;
}

int parse_options(options_t *opts)
{
  /* parse args */
  if(params.num_blks != NULL) {
    opts->num_blks = *params.num_blks;
  }
  if(params.blk_size != NULL) {
    opts->blk_size = *params.blk_size;
  }
  if(params.channel != NULL) {
    ULONG ch = *params.channel;
    if(ch > 15) {
      Printf("Invalid channel given: %ld\n", ch);
      return RETURN_ERROR;
    }
    opts->channel = (UBYTE)ch;
  }
  if(params.operation != NULL) {
    char ch = params.operation[0];
    switch(ch) {
      case 'r':
        opts->operation = OPER_READ;
        break;
      case 'w':
        opts->operation = OPER_WRITE;
        break;
      case 'p':
        opts->operation = OPER_PING;
        break;
      default:
        Printf("Invalid operation: %s\n", params.operation);
        return RETURN_ERROR;
    }
  }
  if(params.port_name != NULL) {
    opts->port_name = params.port_name;
  }
  return RETURN_OK;
}

int dosmain(void)
{
  struct RDArgs *args;

  /* First parse args */
  args = ReadArgs(TEMPLATE, (LONG *)&params, NULL);
  if(args == NULL) {
    PrintFault(IoErr(), "Args Error");
    return RETURN_ERROR;
  }

  /* default args */
  options_t opts;
  opts.num_blks = 10;
  opts.blk_size = 1024;
  opts.channel = 0;
  opts.operation = OPER_WRITE;
  opts.port_name = "parbox.engine";

  int res = RETURN_ERROR;
  if(parse_options(&opts)== RETURN_OK) {
    res = run_client(&opts);
  }

  /* Finally free args */
  FreeArgs(args);
  return res;
}
