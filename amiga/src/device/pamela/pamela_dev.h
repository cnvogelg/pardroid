#ifndef Pamela_DEV_H
#define Pamela_DEV_H

struct PamelaDev {
    struct DevUnitsBase devBase;
};

struct PamelaUnit {
    struct Unit       unit;
    struct DevWorker  worker;
};

#endif
