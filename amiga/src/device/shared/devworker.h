#ifndef DEVWORKER_H
#define DEVWORKER_H

#include <exec/types.h>

struct DevWorker;

typedef BOOL (*InitFunc)(struct DevWorker *worker);
typedef void (*ExitFunc)(struct DevWorker *worker);
typedef BOOL (*HandlerFunc)(struct DevWorker *worker, struct IOStdReq *ior);
typedef BOOL (*SigFunc)(struct DevWorker *worker, ULONG mask);

struct DevWorker
{
    struct SignalSemaphore sem;
    struct MsgPort        *port;
    /* prefill: */
    APTR                   userData;
    InitFunc               initFunc;
    HandlerFunc            handlerFunc;
    ExitFunc               exitFunc;
    /* set in InitFunc(): */
    ULONG                  extraSigMask;
    SigFunc                sigFunc;
};

extern BOOL DevWorkerStart(struct DevWorker *worker, STRPTR name);
extern void DevWorkerStop(struct DevWorker *worker);
extern void DevWorkerBeginIO(struct IOStdReq *ior,
                             struct DevWorker *worker);
extern LONG DevWorkerAbortIO(struct IOStdReq *ior,
                             struct DevWorker *worker);

#endif
