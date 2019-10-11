#ifndef PAMELA_WORKER_H
#define PAMELA_WORKER_H

#include "devworker.h"

extern BOOL pamela_worker_init(struct DevWorker *worker);
extern void pamela_worker_exit(struct DevWorker *worker);
extern void pamela_worker_sig_func(struct DevWorker *worker, ULONG mask);
extern BOOL pamela_worker_open(struct DevWorker *worker, struct IOStdReq *ior, ULONG flags);
extern void pamela_worker_close(struct DevWorker *worker, struct IOStdReq *ior);
extern BOOL pamela_worker_begin_io(struct DevWorker *worker, struct IOStdReq *ior);
extern BOOL pamela_worker_abort_io(struct DevWorker *worker, struct IOStdReq *ior);

#endif
