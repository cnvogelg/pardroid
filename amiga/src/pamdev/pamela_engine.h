#ifndef PAMELA_ENGINE_H
#define PAMELA_ENGINE_H

#include <exec/exec.h>
#include <devices/pamela.h>

/* handle for engine */
struct pamela_engine;
typedef struct pamela_engine pamela_engine_t;

/* pam request */
typedef struct IOPamReq pamela_req_t;

/* ----- API ----- */

/* setup engine */
pamela_engine_t *pamela_engine_init(struct Library *SysBase, int *error);
/* shutdown engine */
void pamela_engine_exit(pamela_engine_t *eng);

/* main worker call of the engine.
   call in an own task in a loop.
   will wait for signals.
   you can add your own sigmask to wait for them, too.
   returns your sigmask with triggered sigmask
   otherwise stays in own processing loop and does not return.
   returns 0 if the quit signal was received.
*/
ULONG pamela_engine_work(pamela_engine_t *eng, ULONG extra_sigmask);

/* send a quit signal to exit the engine running in a task
*/
void pamela_engine_quit(pamela_engine_t *eng);

/* begin request
   setup a new request for a client
*/
int pamela_engine_init_request(pamela_engine_t *eng, pamela_req_t *req);

/* end request
   shutdown a request. cleanup client resources
 */
int pamela_engine_exit_request(pamela_engine_t *eng, pamela_req_t *req);

/* post a read/write.. request
   will reply with the message or return directly
   return TRUE is quick return
*/
BOOL pamela_engine_post_request(pamela_engine_t *eng, pamela_req_t *reg);

/* if possible cancel the request
   return TRUE if it was cancelled
*/
BOOL pamela_engine_cancel_request(pamela_engine_t *eng, pamela_req_t *reg);

#endif
