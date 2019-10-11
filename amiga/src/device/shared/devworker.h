#ifndef DEVWORKER_H
#define DEVWORKER_H

#include <exec/types.h>

struct DevWorker;

typedef BOOL (*InitFunc)(struct DevWorker *worker);
typedef void (*ExitFunc)(struct DevWorker *worker);
typedef BOOL (*ReqFunc)(struct DevWorker *worker, struct IOStdReq *ior);
typedef BOOL (*OpenFunc)(struct DevWorker *worker, struct IOStdReq *ior, ULONG flags);
typedef void (*CloseFunc)(struct DevWorker *worker, struct IOStdReq *ior);
typedef void (*SigFunc)(struct DevWorker *worker, ULONG mask);

struct DevWorker
{
    struct SignalSemaphore sem;
    struct MsgPort        *port;
    /* prefill: */
    APTR                   userData;
    InitFunc               initFunc;
    OpenFunc               openFunc;
    CloseFunc              closeFunc;
    ReqFunc                beginIOFunc;
    ReqFunc                abortIOFunc;
    ExitFunc               exitFunc;
    /* set in InitFunc(): */
    ULONG                  extraSigMask;
    SigFunc                sigFunc;
};

extern BOOL DevWorkerStart(struct DevWorker *worker, STRPTR name, APTR userData);
extern void DevWorkerStop(struct DevWorker *worker);
extern BOOL DevWorkerOpen(struct DevWorker *worker,
                          struct IOStdReq *ior,
                          ULONG flags);
extern void DevWorkerClose(struct DevWorker *worker,
                           struct IOStdReq *ior);
extern void DevWorkerBeginIO(struct DevWorker *worker,
                             struct IOStdReq *ior);
extern LONG DevWorkerAbortIO(struct DevWorker *worker,
                             struct IOStdReq *ior);

#endif
