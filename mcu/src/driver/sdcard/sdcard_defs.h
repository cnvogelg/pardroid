// SD Card Defines

// commands
#define CMD0_GO_IDLE_STATE  0x00
#define CMD1_SEND_OP_COND   0x01
#define CMD8_SEND_IF_COND   0x08
#define CMD_SET_BLOCKLEN    0x10
#define CMD_APP             0x37
#define CMD_READ_OCR        0x3a
#define CMD_CRC_ON_OFF      0x3b

#define SD_SEND_OP_COND     0x29

// status for ready state
#define STATUS_IDLE_STATE   0x01
#define STATUS_CRC_ERROR    0x08
