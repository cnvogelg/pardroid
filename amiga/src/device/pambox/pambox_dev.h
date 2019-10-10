#ifndef PAMBOX_DEV_H
#define PAMBOX_DEV_H

struct PamBoxDev {
    struct DevUnitsBase devBase;
};

struct PamBoxUnit {
    struct Unit       unit;
    struct DevWorker  worker;
};

#endif
