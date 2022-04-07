#include <proto/exec.h>
#include <proto/dos.h>
#include <dos/dos.h>

#define KDEBUG

#include "autoconf.h"
#include "debug.h"
#include "worker.h"

struct my_data {
    STRPTR name;
};

static BOOL init_ok(APTR user_data)
{
    struct my_data *md = (struct my_data *)user_data;

    D(("---> init_ok: %s <---\n", md->name));
    return TRUE;
}

static void work_main(APTR user_data)
{
    struct my_data *md = (struct my_data *)user_data;

    D(("---> main: %s <---\n", md->name));
}

int dosmain(void)
{
    BYTE quit_signal = -1;
    struct my_data md = { "hello!" };

    PutStr("minitest-worker\n");

    struct Task *task = worker_run(
        (struct Library *)SysBase,
        "my_task", 4096,
        init_ok,
        work_main,
        &md, &quit_signal
    );
    Printf("worker_run: task=%ld\n", task);

    if(task != NULL) {
        PutStr("waiting...\n");
        worker_join((struct Library *)SysBase, quit_signal);
        PutStr("done...\n");
    }

    return 0;
}
