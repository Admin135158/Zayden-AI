"""
Zayden-AI — ElMalo Intelligence Consumer
Receives world-state from ElMalo BlackBox via UDP.
No API keys. No network upstream. Pure local mesh.
"""

import socket
import json
import os

ELMALO_PORT = int(os.getenv("ELMALO_PORT", "9164"))

class ElMaloConsumer:
    def __init__(self, port=None):
        self.port = port or ELMALO_PORT
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(("0.0.0.0", self.port))
        self.sock.settimeout(5.0)
        self.last_state = {}

    def listen(self):
        print(f"[Zayden] ElMalo consumer on UDP {self.port}")
        while True:
            try:
                data, addr = self.sock.recvfrom(4096)
                pkt = json.loads(data.decode())
                if pkt.get("protocol") == "elmalo_v1":
                    self.last_state = pkt["payload"]
                    self._process(pkt)
            except socket.timeout:
                continue
            except json.JSONDecodeError:
                continue

    def _process(self, pkt):
        p = pkt["payload"]
        print(f"\n[Zayden] Intelligence from ElMalo")
        print(f"  chaos={p.get('chaos_index'):.2f} order={p.get('order_index'):.2f}")
        print(f"  threat={p.get('threat_level'):.2f} novelty={p.get('novelty_events')}")
        print(f"  theme: {p.get('dominant_theme')}")
        print(f"  signature: {p.get('_attractor_signature')}")
        # Hook into GGSE engine here
        self._update_ggse(p)

    def _update_ggse(self, state):
        # Replace simulated observers with ElMalo data
        pass  # Wire to OpenLoopConsciousnessEngine

    def get_state(self):
        return self.last_state


if __name__ == "__main__":
    ElMaloConsumer().listen()
