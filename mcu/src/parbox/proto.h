#ifndef PROTO_H

// registers
#define MAX_RW_REG          32
#define MAX_RO_REG          32
#define MAX_CHANNEL         16

// command codes
#define CMD_IDLE            0x00
#define CMD_PING            0x10
#define CMD_BOOTLOADER      0x11
#define CMD_RESET           0x1f
#define CMD_MSG_WRITE       0x20
#define CMD_MSG_READ        0x30
#define CMD_RW_REG_WRITE    0x40
#define CMD_RW_REG_READ     0x60
#define CMD_RO_REG_READ     0x80
#define CMD_INVALID         0xff

#define CMD_MASK            0xf0

extern void proto_init(void);
extern void proto_handle(void);

// define these in your code
extern void proto_api_set_rw_reg(u08 reg,u16 val);
extern u16  proto_api_get_rw_reg(u08 reg);
extern u16  proto_api_get_ro_reg(u08 num);

extern u08 *proto_api_prepare_read_msg(u08 chan,u16 *size);
extern void proto_api_done_read_msg(u08 chan);
extern u08 *proto_api_prepare_write_msg(u08 chan,u16 *max_size);
extern void proto_api_done_write_msg(u08 chan,u16 size);

#endif
