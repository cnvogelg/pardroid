#include <proto/exec.h>
#include <proto/alib.h>

#include <exec/types.h>
#include <exec/errors.h>

#include "autoconf.h"

#ifdef CONFIG_DEBUG_DEVICE
#define KDEBUG
#endif

#include "compiler.h"
#include "debug.h"
#include "devworker.h"

#define CMD_TERM 0x7ff0

struct InitData
{
    ULONG initSigMask;
    struct Task *initTask;
    struct DevWorker *worker;
};

static struct InitData *worker_startup(void)
{
    /* retrieve global sys base */
    struct Library *SysBase = *((struct Library **)4);

    struct Task *task = FindTask(NULL);

    return (struct InitData *)task->tc_UserData;
}

static SAVEDS ASM void worker_main(void)
{
    struct IOStdReq *ior;
    struct MsgPort *port;

    /* retrieve dev base stored in user data of task */
    struct InitData *id = worker_startup();
    struct DevWorker *worker = id->worker;
    D(("Task: id=%08lx worker=%08lx\n", id, worker));

    /* create worker port */
    port = CreateMsgPort();

    /* call user init */
    if (port != NULL)
    {
        if (worker->initFunc != NULL)
        {
            if (!worker->initFunc(worker))
            {
                /* user aborted worker */
                DeleteMsgPort(port);
                port = NULL;
            }
        }
    }

    /* setup port or NULL and trigger signal to caller task */
    worker->port = port;
    D(("Task: signal task=%08lx mask=%08lx\n", id->initTask, id->initSigMask));
    Signal(id->initTask, id->initSigMask);

    /* only if port is available then enter work loop. otherwise quit task */
    if (port != NULL)
    {
        /* define workers sig mask */
        ULONG portMask = 1UL << port->mp_SigBit;
        ULONG extraMask = worker->extraSigMask;
        ULONG sigMask = portMask | extraMask;

        /* worker loop */
        D(("Task: enter\n"));
        BOOL stay = TRUE;
        while (stay)
        {
            ULONG signals = Wait(sigMask);

            /* handle extra signals */
            if((signals & extraMask) != 0) {
                worker->sigFunc(worker, signals & extraMask);
            }

            /* got messages */
            if((signals & portMask) == portMask) {
                while (1)
                {
                    ior = (struct IOStdReq *)GetMsg(port);
                    if (ior == NULL)
                    {
                        break;
                    }
                    /* terminate? */
                    if (ior->io_Command == CMD_TERM)
                    {
                        stay = FALSE;
                        ReplyMsg(&ior->io_Message);
                        break;
                    }
                    /* regular command */
                    else
                    {
                        ObtainSemaphore(&worker->sem);
                        BOOL done = worker->handlerFunc(worker, ior);
                        ReleaseSemaphore(&worker->sem);
                        if (done)
                        {
                            ReplyMsg(&ior->io_Message);
                        }
                    }
                }
            }
        }

        /* call shutdown only if worker was entered */
        D(("Task: exit\n"));
        /* shutdown worker */
        worker->exitFunc(worker);
    }

    D(("Task: delete port\n"));
    DeleteMsgPort(port);
    worker->port = NULL;

    /* kill myself */
    D(("Task: die\n"));
    struct Task *me = FindTask(NULL);
    DeleteTask(me);
    Wait(0);
    D(("Task: NEVER!\n"));
}

BOOL DevWorkerStart(struct DevWorker *worker, STRPTR taskName)
{
    D(("Worker: start\n"));
    worker->port = NULL;

    /* alloc a signal */
    BYTE signal = AllocSignal(-1);
    if (signal == -1)
    {
        D(("Worker: NO SIGNAL!\n"));
        return FALSE;
    }

    /* setup init data */
    struct InitData id;
    id.initSigMask = 1 << signal;
    id.initTask = FindTask(NULL);
    id.worker = worker;
    D(("Worker: init data %08lx\n", &id));

    /* now launch worker task and inject dev base
     make sure worker_main() does not run before base is set.
  */
    Forbid();
    struct Task *myTask = CreateTask(taskName, 0, (CONST APTR)worker_main, 4096);
    if (myTask != NULL)
    {
        myTask->tc_UserData = (APTR)&id;
    }
    Permit();
    if (myTask == NULL)
    {
        D(("Worker: NO TASK!\n"));
        FreeSignal(signal);
        return FALSE;
    }

    /* wait for start signal of new task */
    D(("Worker: wait for task startup. sigmask=%08lx\n", id.initSigMask));
    Wait(id.initSigMask);

    FreeSignal(signal);

    /* ok everything is fine. worker is ready to receive commands */
    D(("Worker: started: port=%08lx\n", worker->port));
    return (worker->port != NULL) ? TRUE : FALSE;
}

void DevWorkerStop(struct DevWorker *worker)
{
    struct IORequest newior;

    D(("Worker: stop\n"));

    if (worker->port != NULL)
    {
        /* send a message to the child process to shut down. */
        newior.io_Message.mn_ReplyPort = CreateMsgPort();
        newior.io_Command = CMD_TERM;

        /* send term message and wait for reply */
        PutMsg(worker->port, &newior.io_Message);
        WaitPort(newior.io_Message.mn_ReplyPort);
        DeleteMsgPort(newior.io_Message.mn_ReplyPort);
        worker->port = NULL;
    }

    D(("Worker: stopped\n"));
}

void DevWorkerBeginIO(struct IOStdReq *ior,
                      struct DevWorker *worker)
{
    /* clear error */
    ior->io_Error = 0;

    /* can we handle the request directly? */
    if (AttemptSemaphore(&worker->sem))
    {
        BOOL done = worker->handlerFunc(worker, ior);
        ReleaseSemaphore(&worker->sem);
        if (done)
        {
            /* need to reply? */
            if ((ior->io_Flags & IOF_QUICK) == 0)
            {
                ReplyMsg((struct Message *)ior);
            }
        }
        else
        {
            /* make sure its no quick request */
            ior->io_Flags &= ~IOF_QUICK;
        }
    }
    /* unit is busy we need to queue the request */
    else
    {
        ior->io_Flags &= ~IOF_QUICK;
        PutMsg(worker->port, (struct Message *)ior);
    }
}

LONG DevWorkerAbortIO(struct IOStdReq *ior,
                      struct DevWorker *worker)
{
    LONG result = 1;

    ObtainSemaphore(&worker->sem);
    if((ior->io_Message.mn_Node.ln_Type==NT_MESSAGE) &&
        ((ior->io_Flags&IOF_QUICK)==0))
    {
      Remove((struct Node *)ior);
      ReplyMsg((struct Message *)ior);
      ior->io_Error=IOERR_ABORTED;
      result = 0;
    } 
    ReleaseSemaphore(&worker->sem);
    return result;
}
