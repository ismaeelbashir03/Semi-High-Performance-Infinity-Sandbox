import socket

from config import HOST, PORT, WIDTH, HEIGHT
from protocol import pack_init


def recv_exact(sock, buffer):
    view = memoryview(buffer)
    remaining = len(buffer)
    while remaining:
        received = sock.recv_into(view, remaining)
        if received == 0:
            raise ConnectionError("Socket closed")
        view = view[received:]
        remaining -= received


class NetworkClient:
    def __init__(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.sock.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)

    def connect(self):
        self.sock.connect((HOST, PORT))
        self.sock.sendall(pack_init(WIDTH, HEIGHT))

    def send_and_receive(self, payload, buffer):
        self.sock.sendall(payload)
        recv_exact(self.sock, buffer)

    def close(self):
        self.sock.close()
