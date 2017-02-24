#ifndef PROTO_H

// registers
#define NUM_REG             16
#define NUM_CHANNEL         16
#define REG_TEST            0

// command codes
#define CMD_IDLE            0x00
#define CMD_PING            0x10
#define CMD_BOOTLOADER      0x11
#define CMD_RESET           0x1f
#define CMD_MSG_WRITE       0x20
#define CMD_MSG_READ        0x30
#define CMD_REG_WRITE       0x40
#define CMD_REG_READ        0x50
#define CMD_CONST_READ      0x60
#define CMD_INVALID         0xff

#define CMD_MASK            0xf0

extern void proto_init(void);
extern void proto_handle(void);

// define these in your code
extern void proto_api_set_reg(u08 reg,u16 val);
extern u16  proto_api_get_reg(u08 reg);
extern u16  proto_api_get_const(u08 num);
extern u08 *proto_api_get_read_msg(u16 *size);
extern u08 *proto_api_get_write_msg(u16 *max_size);
extern void proto_api_set_write_msg_size(u16 size);

#endif
