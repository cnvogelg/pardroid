#ifndef PAMELA_SHARED_H
#define PAMELA_SHARED_H

/* pamela channel status bits */

// first nibble is the state byte
#define PAMELA_STATUS_STATE_MASK  0x0f

// -- passive states (bit 3 clear)
// inactive
#define PAMELA_STATUS_INACTIVE    0x00
// open error: open command failed
#define PAMELA_STATUS_OPEN_ERROR  0x01
// eos: reached end of stream and needs to be closed
#define PAMELA_STATUS_EOS         0x02
// error: stream has error and needs to be closed or reset
#define PAMELA_STATUS_ERROR       0x03

// -- active states (bit 3 set)
// MASK: active states
#define PAMELA_STATUS_ACTIVE_MASK 0x08
// active: open successful and channel operational
#define PAMELA_STATUS_ACTIVE      0x08
// opening: open command is being processed
#define PAMELA_STATUS_OPENING     0x09
// closing: close command is being processed
#define PAMELA_STATUS_CLOSING     0x0a
// resetting: stream will be reset
#define PAMELA_STATUS_RESETTING   0x0b

// -- extra bits for read in active
// read pending: a read request is ready to be retrieved
#define PAMELA_STATUS_READ_READY  0x10
// read size differs from request and has to be read first
#define PAMELA_STATUS_READ_SIZE   0x20
// read requested: a read request was posted and is now being processed
#define PAMELA_STATUS_READ_BUSY   0x40
// mask for all read bits
#define PAMELA_STATUS_READ_MASK   0x70

// -- extra bits for write in active
// write pending: a write request is ready to be sent
#define PAMELA_STATUS_WRITE_READY 0x100
// write size differs from request and has to be read first
#define PAMELA_STATUS_WRITE_SIZE  0x200
// write requested: a write request was posted and is now being processed
#define PAMELA_STATUS_WRITE_BUSY  0x400
// mask for all write bits
#define PAMELA_STATUS_WRITE_MASK  0x700

// ----- error -----
// error tags allow to distinguish error sources (e.g. pamela or application)
#define PAMELA_WIRE_ERROR_TAG_PAMELA   0x0000
#define PAMELA_WIRE_ERROR_TAG_APP      0x8000
#define PAMELA_WIRE_ERROR_TAG_MASK     0xc000

// -- pamela errors --
#define PAMELA_WIRE_OK                    0
// no handler/service found for given port
#define PAMELA_WIRE_ERROR_NO_SERVICE      1
// no empty slot in service found for this channel
#define PAMELA_WIRE_ERROR_NO_SLOT         2
// generic open error
#define PAMELA_WIRE_ERROR_OPEN            3
// generic reset error
#define PAMELA_WIRE_ERROR_RESET           4
// generic close error
#define PAMELA_WIRE_ERROR_CLOSE           5
// generic read error
#define PAMELA_WIRE_ERROR_READ            6
// general write error
#define PAMELA_WIRE_ERROR_WRITE           7
// no memory
#define PAMELA_WIRE_ERROR_NO_MEM          8
// wrong state
#define PAMELA_WIRE_ERROR_WRONG_STATE     9
// object not found
#define PAMELA_WIRE_ERROR_OBJ_NOT_FOUND   10
// not supported
#define PAMELA_WIRE_ERROR_NOT_SUPPORTED   11
// wrong size
#define PAMELA_WIRE_ERROR_WRONG_SIZE      12

#endif
