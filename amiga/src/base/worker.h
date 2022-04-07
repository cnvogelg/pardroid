#ifndef WORKER_H
#define WORKER_H

#include <exec/types.h>

typedef BOOL (*worker_init_func_t)(APTR user_data);
typedef void (*worker_main_func_t)(APTR user_data);

/* launch a task, run the init func there and if it
   succeeds enter the main func.

   it is guaranteed that the init functions ran
   before return.

   if quit signal != NULL then a join signal is allocated
   it will be sent after leaving main func.
   use worker_join to wait for the signal (and free it)

   returns task if main is running or NULL
*/
extern struct Task *worker_run(struct Library *sys_base,
  STRPTR task_name, ULONG stack_size,
  worker_init_func_t init_func,
  worker_main_func_t main_func,
  APTR user_data,
  BYTE *quit_signal);

/* wait in the task that issued run until the quit signal
   occurred. also free the signal */
extern void worker_join(struct Library *sys_base, BYTE quit_signal);

#endif
