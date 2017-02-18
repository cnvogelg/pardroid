#ifndef PROTO_H

// registers
#define NUM_REG             16
#define REG_TEST            0

// command codes
#define CMD_IDLE            0x00
#define CMD_PING            0x10
#define CMD_RESET           0x1f
// registers are located in 0x50 - 0x6f
// read range is +0x80: 0xd0 - 0xef
#define CMD_REG_WRITE_BASE  0x50
#define CMD_REG_WRITE_LAST  0x6f
#define CMD_REG_READ_BASE   0xd0
#define CMD_REG_READ_LAST   0xef
#define CMD_INVALID         0xff

extern void proto_init(void);
extern void proto_handle(void);

#endif
