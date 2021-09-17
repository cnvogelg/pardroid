import socket
import struct


MAGIC = b'APB\0'
SIM_PKT_PROTO_CONNECT      = 1
SIM_PKT_PROTO_CON_ACK      = 2
SIM_PKT_PROTO_DISCONNECT   = 3
SIM_PKT_PROTO_DIS_ACK      = 4
SIM_PKT_PROTO_CMD_REQ      = 5
SIM_PKT_PROTO_CMD_REP      = 6
SIM_PKT_PROTO_STATUS       = 7
SIM_PKT_PROTO_SRV_ALIVE    = 8

SIM_PKT_STATUS_ACK_IRQ     = 2


class SimPkt:
    def __init__(self, type, value=0, data=None):
        self.type = type
        self.value = value
        self.data = data

    def __repr__(self):
        return "SimPkt(%d,%d,%r)" % (self.type, self.value, self.data)

    def encode(self):
        if self.data:
            size = len(self.data)
        else:
            size = 0
        buf = struct.pack(">4sBBH", MAGIC, self.type, self.value, size)
        if self.data:
            buf += self.data
        return buf

    @classmethod
    def decode(cls, buf):
        n = len(buf)
        assert n >= 8
        magic, type, value, size = struct.unpack(">4sBBH", buf[0:8])
        assert magic == MAGIC
        if n > 8:
            data = buf[8:]
        else:
            data = None
        return cls(type, value, data)


class SimPktClient:
    def __init__(self, peer_addr, time_out=1, buf_size=4096):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.settimeout(time_out)
        self.peer_addr = peer_addr
        self.buf_size = buf_size
        self.status = 0
        self.alive_missing = 0

    def _send(self, sim_pkt):
        data = sim_pkt.encode()
        self.sock.sendto(data, self.peer_addr)

    def recv_cmd(self):
        sim_pkt = self._recv()
        if sim_pkt:
            assert sim_pkt.type == SIM_PKT_PROTO_CMD_REQ
            return sim_pkt.type, sim_pkt.data
        else:
            return -1, None

    def reply_cmd(self, cmd, data=None):
        reply = SimPkt(SIM_PKT_PROTO_CMD_REP, cmd, data)
        self._send(reply)

    def _recv(self, retries=1, send_status=True):
        for _ in range(retries):
            try:
                data, peer_addr = self.sock.recvfrom(self.buf_size)
                sim_pkt = SimPkt.decode(data)
                self.alive_missing = 0
                # ignore alive packets
                if sim_pkt.type != SIM_PKT_PROTO_SRV_ALIVE:
                    return sim_pkt
            except socket.timeout:
                # client alive with status
                if send_status:
                    self._send_status()
        # check for server alive
        self.alive_missing += 1
        if self.alive_missing > 5:
            raise IOError("server not alive?")

    def _send_status(self):
        # send status
        sim_pkt = SimPkt(SIM_PKT_PROTO_STATUS, self.status)
        self._send(sim_pkt)

    def connect(self):
        sim_pkt = SimPkt(SIM_PKT_PROTO_CONNECT)
        self._send(sim_pkt)
        # get response
        ret_pkt = self._recv(retries=5, send_status=False)
        if not ret_pkt:
            raise IOError("no connect reply packet? server down?")
        if ret_pkt.type != SIM_PKT_PROTO_CON_ACK:
            raise IOError("no connect reply: wrong packet?")

    def disconnect(self):
        sim_pkt = SimPkt(SIM_PKT_PROTO_DISCONNECT)
        self._send(sim_pkt)
        # get response
        ret_pkt = self._recv(send_status=False)
        assert ret_pkt
        assert ret_pkt.type == SIM_PKT_PROTO_DIS_ACK
        # close socket
        self.sock.close()

    def set_status(self, status):
        self.status = status
        self._send_status()

    def set_ack_irq(self, on):
        if on:
            self.status |= SIM_PKT_STATUS_ACK_IRQ
        else:
            self.status &= ~SIM_PKT_STATUS_ACK_IRQ
        self._send_status()
