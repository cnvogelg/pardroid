#ifndef PAMELA_SHARED_H
#define PAMELA_SHARED_H

/* pamela channel status bits */
// open: channel is open
#define PAMELA_STATUS_OPEN        0x01
// end of stream reached and needs to be closed or reset
#define PAMELA_STATUS_EOS         0x02
// stream has error and needs to be closed or reset
#define PAMELA_STATUS_ERROR       0x04

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

#endif
