/* common device commands shared by bootloader and pamela */
#ifndef PROTO_DEV_SHARED_H
#define PROTO_DEV_SHARED_H

/* command range 0x10..0x1f is reserved for device commands */

/* actions to trigger device mode changes */
#define PROTO_DEV_CMD_ACTION_PING         0x10
#define PROTO_DEV_CMD_ACTION_BOOTLOADER   0x11
#define PROTO_DEV_CMD_ACTION_RESET        0x12
#define PROTO_DEV_CMD_ACTION_KNOK         0x13

/* read device constants */
#define PROTO_DEV_CMD_RWORD_FW_ID         0x18
#define PROTO_DEV_CMD_RWORD_FW_VERSION    0x19
#define PROTO_DEV_CMD_RWORD_MACH_TAG      0x1a
#define PROTO_DEV_CMD_RWORD_DRIVER_TOKEN  0x1b

/* write commands */
#define PROTO_DEV_CMD_WWORD_DRIVER_TOKEN  0x1f

#endif