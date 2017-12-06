#ifndef WIZ_DEFS_H
#define WIZ_DEFS_H

#define WIZ_MAX_SOCKETS             8

#define WIZ_REG_BASE_MODE           0x00
#define WIZ_REG_BASE_GW_ADDR        0x01
#define WIZ_REG_BASE_NET_MASK       0x05
#define WIZ_REG_BASE_SRC_MAC        0x09
#define WIZ_REG_BASE_SRC_ADDR       0x0f
#define WIZ_REG_BASE_PHY_STATUS     0x35

#define WIZ_MASK_RESET              0x80
#define WIZ_MASK_LINK_UP            0x20

#define WIZ_REG_SOCKET_MODE         0x00
#define WIZ_REG_SOCKET_CMD          0x01
#define WIZ_REG_SOCKET_IR           0x02
#define WIZ_REG_SOCKET_STATUS       0x03
#define WIZ_REG_SOCKET_SRC_PORT     0x04
#define WIZ_REG_SOCKET_DST_MAC      0x06
#define WIZ_REG_SOCKET_DST_IP       0x0c
#define WIZ_REG_SOCKET_DST_PORT     0x10
#define WIZ_REG_SOCKET_MSS          0x12
#define WIZ_REG_SOCKET_PROTO        0x14
#define WIZ_REG_SOCKET_TOS          0x15
#define WIZ_REG_SOCKET_TTL          0x16

#define WIZ_REG_SOCKET_RX_MEMSIZE   0x1e
#define WIZ_REG_SOCKET_TX_MEMSIZE   0x1f
#define WIZ_REG_SOCKET_TX_FREE      0x20
#define WIZ_REG_SOCKET_TX_READ_PTR  0x22
#define WIZ_REG_SOCKET_TX_WRITE_PTR 0x24
#define WIZ_REG_SOCKET_RX_SIZE      0x26
#define WIZ_REG_SOCKET_RX_READ_PTR  0x28
#define WIZ_REG_SOCKET_RX_WRITE_PTR 0x2a

#define WIZ_REG_MEMSIZE_0KB         0x00
#define WIZ_REG_MEMSIZE_1KB         0x01
#define WIZ_REG_MEMSIZE_2KB         0x02
#define WIZ_REG_MEMSIZE_4KB         0x04
#define WIZ_REG_MEMSIZE_8KB         0x08
#define WIZ_REG_MEMSIZE_16KB        0x10

#define WIZ_SOCKET_MODE_MASK        0x0f
#define WIZ_SOCKET_MODE_CLOSED      0x00
#define WIZ_SOCKET_MODE_TCP         0x01
#define WIZ_SOCKET_MODE_UDP         0x02
#define WIZ_SOCKET_MODE_IPRAW       0x03
#define WIZ_SOCKET_MODE_MACRAW      0x04
#define WIZ_SOCKET_MODE_PPPOE       0x05

#define WIZ_SOCKET_CMD_NONE         0x00
#define WIZ_SOCKET_CMD_OPEN         0x01
#define WIZ_SOCKET_CMD_LISTEN       0x02
#define WIZ_SOCKET_CMD_CONNECT      0x04
#define WIZ_SOCKET_CMD_DISCONNECT   0x08
#define WIZ_SOCKET_CMD_CLOSE        0x10
#define WIZ_SOCKET_CMD_SEND         0x20
#define WIZ_SOCKET_CMD_SEND_MAC     0x21
#define WIZ_SOCKET_CMD_SEND_KEEP    0x22
#define WIZ_SOCKET_CMD_RECV         0x40

#define WIZ_SOCKET_IR_SEND_OK       0x10
#define WIZ_SOCKET_IR_TIMEOUT       0x08
#define WIZ_SOCKET_IR_RECV          0x04
#define WIZ_SOCKET_IR_DISCONNECT    0x02
#define WIZ_SOCKET_IR_CONNECT       0x01

#define WIZ_SOCKET_STATUS_CLOSED        0x00
#define WIZ_SOCKET_STATUS_INIT          0x13
#define WIZ_SOCKET_STATUS_LISTEN        0x14
#define WIZ_SOCKET_STATUS_ESTABLISHED   0x17
#define WIZ_SOCKET_STATUS_CLOSE_WAIT    0x1c
#define WIZ_SOCKET_STATUS_UDP           0x22
#define WIZ_SOCKET_STATUS_IPRAW         0x32
#define WIZ_SOCKET_STATUS_MACRAW        0x42
#define WIZ_SOCKET_STATUS_PPPOE         0x5f

#define WIZ_SOCKET_STATUS_SYN_SEND      0x15
#define WIZ_SOCKET_STATUS_SYN_RECV      0x16
#define WIZ_SOCKET_STATUS_FIN_WAIT      0x18
#define WIZ_SOCKET_STATUS_CLOSING       0x1a
#define WIZ_SOCKET_STATUS_TIME_WAIT     0x1b
#define WIZ_SOCKET_STATUS_LAST_ACK      0x1d
#define WIZ_SOCKET_STATUS_ARP           0x01

#endif
