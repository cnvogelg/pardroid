#include <proto/exec.h>
#include <stdio.h>

#include "autoconf.h"
#include "debug.h"
#include "pario.h"

int main(int argc, char **argv)
{
    struct pario_handle *ph;

    printf("parbox-test!\n");
    D(("pario_init\n"));
    ph = pario_init((struct Library *)SysBase);
    if(ph != NULL) {
        D(("pario ok!\n"));
        pario_exit(ph);
    } else {
        printf("error setting up pario!\n");
    }
    return 0;
}
