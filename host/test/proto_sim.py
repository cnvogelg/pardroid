#!/usr/bin/env python3
# test udp server

import socket
import struct

PROTO_CMD_MASK                          = 0xf0
PROTO_CHAN_MASK                         = 0x0f

PROTO_CMD_ACTION                        = 0x10
PROTO_CMD_WFUNC_READ                    = 0x20
PROTO_CMD_WFUNC_WRITE                   = 0x30
PROTO_CMD_LFUNC_READ                    = 0x40
PROTO_CMD_LFUNC_WRITE                   = 0x50
PROTO_CMD_CHN_READ_DATA                 = 0x60
PROTO_CMD_CHN_WRITE_DATA                = 0x70
PROTO_CMD_CHN_GET_RX_SIZE               = 0x80
PROTO_CMD_CHN_SET_TX_SIZE               = 0x90
PROTO_CMD_CHN_SET_OFFSET                = 0xa0
PROTO_CMD_CHN_CANCEL_TRANSFER           = 0xb0

PROTO_ACTION_PING                       = 0x00
PROTO_ACTION_BOOTLOADER                 = 0x01
PROTO_ACTION_RESET                      = 0x02
PROTO_ACTION_KNOK                       = 0x03

PROTO_ACTION_TEST_SIGNAL                = 0x04
PROTO_ACTION_TEST_BUSY_LOOP             = 0x05

TEST_WORD = 0
TEST_LONG = 0
TEST_SIZE = 0
TEST_OFFSET = 0
TEST_DATA = b''


def recv_cmd(sock, seq):
    data, peer_addr = sock.recvfrom(4096)
    if len(data) < 8:
        raise IOError("Packet too small!")
    tag, cmd, blk_seq = struct.unpack(">3sBI", data[0:8])
    if tag != b'APB':
        raise IOError("No APB header!")
    if blk_seq != seq:
        raise IOError("Sequence mismatch: got=%d want=%d" % (blk_seq, seq))
    if len(data) > 8:
        ret_data = data[8:]
    else:
        ret_data = None
    return peer_addr, cmd, ret_data


def reply_cmd(sock, seq, peer_addr, cmd, data):
    hdr = struct.pack(">3sBI", b'APB', cmd, seq)
    if data:
        buf = hdr + data
    else:
        buf = hdr
    sock.sendto(buf, peer_addr)


def handle_action(what):
    if what == PROTO_ACTION_PING:
        print("PING")
    elif what == PROTO_ACTION_BOOTLOADER:
        print("BOOTLOADER")
    elif what == PROTO_ACTION_RESET:
        print("RESET")
    elif what == PROTO_ACTION_KNOK:
        print("KNOK")
    elif what == PROTO_ACTION_TEST_BUSY_LOOP:
        print("BUSY LOOP")
    elif what == PROTO_ACTION_TEST_SIGNAL:
        print("SIGNAL")
    else:
        print("UNKOWN ACTION", what)


def handle_wfunc_read(chn):
    global TEST_WORD
    print(chn, ": WFUNC READ", TEST_WORD)
    return struct.pack(">H", TEST_WORD)


def handle_wfunc_write(chn, data):
    global TEST_WORD
    word = struct.unpack(">H", data)[0]
    print(chn, ": WFUNC WRITE", word)
    TEST_WORD = word


def handle_lfunc_read(chn):
    global TEST_LONG
    print(chn, ": LFUNC READ", TEST_LONG)
    return struct.pack(">I", TEST_LONG)


def handle_lfunc_write(chn, data):
    global TEST_LONG
    long = struct.unpack(">I", data)[0]
    print(chn, ": LFUNC WRITE", long)
    TEST_LONG = long


def handle_read_data(chn):
    global TEST_DATA
    size = len(TEST_DATA) if TEST_DATA else 0
    print(chn, ": READ DATA", size, TEST_DATA)
    return TEST_DATA


def handle_write_data(chn, data):
    global TEST_DATA
    TEST_DATA = data
    size = len(TEST_DATA) if TEST_DATA else 0
    print(chn, ": WRITE DATA", size, TEST_DATA)


def handle_get_rx_size(chn):
    global TEST_SIZE
    print(chn, ": GET RX SIZE", TEST_SIZE)
    return struct.pack(">H", TEST_SIZE)


def handle_set_tx_size(chn, data):
    global TEST_SIZE
    size = struct.unpack(">H", data)[0]
    print(chn, ": SET TX SIZE", size)
    TEST_SIZE = size


def handle_set_offset(chn, data):
    global TEST_OFFSET
    offset = struct.unpack(">I", data)[0]
    print(chn, ": SET OFFSET", offset)
    TEST_OFFSET = offset


def handle_cmd(cmd, data):
    grp = cmd & PROTO_CMD_MASK
    chn = cmd & PROTO_CHAN_MASK
    if grp == PROTO_CMD_ACTION:
        handle_action(chn)
    elif grp == PROTO_CMD_WFUNC_READ:
        return handle_wfunc_read(chn)
    elif grp == PROTO_CMD_WFUNC_WRITE:
        return handle_wfunc_write(chn, data)
    elif grp == PROTO_CMD_LFUNC_READ:
        return handle_lfunc_read(chn)
    elif grp == PROTO_CMD_LFUNC_WRITE:
        return handle_lfunc_write(chn, data)
    elif grp == PROTO_CMD_CHN_READ_DATA:
        return handle_read_data(chn)
    elif grp == PROTO_CMD_CHN_WRITE_DATA:
        return handle_write_data(chn, data)
    elif grp == PROTO_CMD_CHN_GET_RX_SIZE:
        return handle_get_rx_size(chn)
    elif grp == PROTO_CMD_CHN_SET_TX_SIZE:
        return handle_set_tx_size(chn, data)
    elif grp == PROTO_CMD_CHN_SET_OFFSET:
        return handle_set_offset(chn, data)
    else:
        print("Unknown cmd: %02x" % cmd)
    return None


def main():
    addr = ('localhost', 2001)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(addr)

    print("waiting...")
    seq = 0
    while True:
        peer_addr, cmd, data = recv_cmd(sock, seq)
        result = handle_cmd(cmd, data)
        reply_cmd(sock, seq, peer_addr, cmd, result)
        seq += 1


if __name__ == '__main__':
    main()
