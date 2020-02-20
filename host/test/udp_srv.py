#!/usr/bin/env python3
# test udp server

import socket

def main():
    addr = ('localhost', 1234)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind(addr)

    print("waiting...")
    data, peer_addr = sock.recvfrom(4096)
    print("got", len(data), "from", peer_addr)
    print(data)

    print("reply")
    sock.sendto(data, peer_addr)
    print("done")

if __name__ == '__main__':
    main()
