#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#include <string.h>
#include <stdio.h>

#include "autoconf.h"
#include "compiler.h"
#include "debug.h"

#include "engine.h"

static const char *TEMPLATE =
   "P=PortName/N";
typedef struct {
  char *port_name;
} params_t;
static params_t params;

int run_engine(const char *port_name)
{
  Printf("Running parbox engine on port '%s'\n", port_name);

  int result = 0;
  engine_handle_t *eh = engine_start(&result, (struct Library *)SysBase, port_name);
  if(eh != NULL) {

    PutStr("Press Ctrl+C to stop engine...\n");
    Wait(SIGBREAKF_CTRL_C);

    engine_stop(eh);
    PutStr("done\n");
  } else {
    Printf("Staring engine failed! result=%ld -> %s\n", result, engine_perror(result));
    return RETURN_ERROR;
  }
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

  /* set port name */
  char *port_name = "parbox.engine";
  if(params.port_name != NULL) {
    port_name = params.port_name;
  }

  int res = run_engine(port_name);

  /* Finally free args */
  FreeArgs(args);
  return res;
}
