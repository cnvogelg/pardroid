#ifndef PAMELA_DEFS_H
#define PAMELA_DEFS_H

/* pamela init error codes */
#define PAMELA_OK                        0
#define PAMELA_ERROR_NO_MEM              -1
#define PAMELA_ERROR_INIT_ENV            -2
#define PAMELA_ERROR_INIT_PROTO          -3
#define PAMELA_ERROR_PROTO_RAK_INVALID   -4
#define PAMELA_ERROR_PROTO_TIMEOUT       -5
#define PAMELA_ERROR_PROTO_DEV_BUSY      -6
#define PAMELA_ERROR_PROTO_ODD_SIZE      -7
#define PAMELA_ERROR_NO_FREE_CHANNEL     -8
#define PAMELA_ERROR_DEV_OPEN_FAILED     -9
#define PAMELA_ERROR_CHANNEL_NOT_OPEN    -10
#define PAMELA_ERROR_CHANNEL_EOS         -11
#define PAMELA_ERROR_CHANNEL_ERROR       -12
#define PAMELA_ERROR_CHANNEL_STATE       -13
#define PAMELA_ERROR_MSG_TOO_LARGE       -14
#define PAMELA_ERROR_UNKNOWN             -99

/* wait event result */
#define PAMELA_WAIT_TIMEOUT              1
#define PAMELA_WAIT_SIGMASK              2
#define PAMELA_WAIT_EVENT                4

/* pamela channel status bits */
// open: channel is open
#define PAMELA_CHANNEL_OPEN        0x01
// end of stream reached and needs to be closed or reset
#define PAMELA_CHANNEL_EOS         0x02
// stream has error and needs to be closed or reset
#define PAMELA_CHANNEL_ERROR       0x04

// read pending: a read request is ready to be retrieved
#define PAMELA_CHANNEL_READ_READY  0x10
// read size differs from request and has to be read first
#define PAMELA_CHANNEL_READ_SIZE   0x20
// read requested: a read request was posted
#define PAMELA_CHANNEL_READ_REQ    0x40
// read requested was aborted by device
#define PAMELA_CHANNEL_READ_ERROR  0x80
// mask for all read bits
#define PAMELA_CHANNEL_READ_MASK   0xF0

// write pending: a write request is ready to be sent
#define PAMELA_CHANNEL_WRITE_READY 0x100
// write size differs from request and has to be read first
#define PAMELA_CHANNEL_WRITE_SIZE  0x200
// write requested: a write request was posted
#define PAMELA_CHANNEL_WRITE_REQ   0x400
// read requested was aborted by device
#define PAMELA_CHANNEL_WRITE_ERROR 0x800
// mask for all write bits
#define PAMELA_CHANNEL_WRITE_MASK  0xF00

/* check if read req is done */
static inline int pamela_status_read_ready(UWORD status)
{
  return (status & PAMELA_CHANNEL_READ_READY) == PAMELA_CHANNEL_READ_READY;
}

/* check if write req is done */
static inline int pamela_status_write_ready(UWORD status)
{
  return (status & PAMELA_CHANNEL_WRITE_READY) == PAMELA_CHANNEL_WRITE_READY;
}

/* device information */
struct pamela_devinfo {
  UWORD     firmware_id;
  UWORD     firmware_version;
  UWORD     mach_tag;
  UWORD     default_mtu;
  UWORD     max_channels;
};
typedef struct pamela_devinfo pamela_devinfo_t;

#endif
