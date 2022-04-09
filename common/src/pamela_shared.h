#ifndef PAMELA_SHARED_H
#define PAMELA_SHARED_H

/* pamela channel status bits */

// first nibble is the state byte
#define PAMELA_STATUS_STATE_MASK  0x0f
// inactive
#define PAMELA_STATUS_INACTIVE    0x00
// opening: open command is being processed
#define PAMELA_STATUS_OPENING     0x01
// active: open successful and channel operational
#define PAMELA_STATUS_ACTIVE      0x02
// closing: close command is being processed
#define PAMELA_STATUS_CLOSING     0x03
// eos: reached end of stream and needs to be closed
#define PAMELA_STATUS_EOS         0x04
// error: stream has error and needs to be closed or reset
#define PAMELA_STATUS_ERROR       0x05
// resetting: stream will be reset
#define PAMELA_STATUS_RESETTING   0x06

// read pending: a read request is ready to be retrieved
#define PAMELA_STATUS_READ_READY  0x10
// read size differs from request and has to be read first
#define PAMELA_STATUS_READ_SIZE   0x20
// read requested: a read request was posted
#define PAMELA_STATUS_READ_REQ    0x40
// read requested was aborted by device
#define PAMELA_STATUS_READ_ERROR  0x80
// mask for all read bits
#define PAMELA_STATUS_READ_MASK   0xF0

// write pending: a write request is ready to be sent
#define PAMELA_STATUS_WRITE_READY 0x100
// write size differs from request and has to be read first
#define PAMELA_STATUS_WRITE_SIZE  0x200
// write requested: a write request was posted
#define PAMELA_STATUS_WRITE_REQ   0x400
// read requested was aborted by device
#define PAMELA_STATUS_WRITE_ERROR 0x800
// mask for all write bits
#define PAMELA_STATUS_WRITE_MASK  0xF00

// top nibble is the error code if any or 0
#define PAMELA_STATUS_ERROR_MASK  0xf000
// shift for error byte
#define PAMELA_STATUS_ERROR_SHIFT 12

// ----- error -----
#define PAMELA_DEV_ERR_NO_SERVICE   1

#endif
