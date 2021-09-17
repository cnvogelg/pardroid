#!/usr/bin/env python3
# pario_sim.py - client for pario_sim on Amiga side

import simpkt


def main():
    peer_addr = ("localhost", 1234)
    client = simpkt.SimPktClient(peer_addr, 1, 4096)
    client.connect()
    print("connected...")

    ack_irq = True
    ack_wait = 0
    try:
        while True:
            cmd, data = client.recv_cmd()
            if cmd >= 0:
                print("CMD", cmd, data)
                client.reply_cmd(cmd, b"hello, world!")
            else:
                ack_wait += 1
                if ack_wait == 10:
                    ack_wait = 0
                    print("ack_irq", ack_irq)
                    client.set_ack_irq(ack_irq)
                    ack_irq = not ack_irq
    except KeyboardInterrupt:
        print("bye...")

    client.disconnect()
    print("disconnected...")


if __name__ == '__main__':
    main()
