#ifndef DISK_DEV_H
#define DISK_DEV_H

/* --- disk devices --- */
/* special device to denote no device assigned now */
#define DISK_DEVICE_NONE               0
/* special device to denote that the actual device is found at another disk service */
#define DISK_DEVICE_REDIRECT           0xff
/* test device */
#define DISK_DEVICE_TEST               1

/* device mapping image files from sd card */
#define DISK_DEVICE_SD_IMG_FILE        2

#endif
