#ifndef PROTO_H

#define CMD_IDLE        0x00
#define CMD_PING        0x10
#define CMD_TEST_READ   0x11
#define CMD_TEST_WRITE  0x12
#define CMD_INVALID     0xff

extern void proto_init(void);
extern void proto_handle(void);

#endif
