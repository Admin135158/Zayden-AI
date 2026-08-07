#!/usr/bin/env python3
import socket
import sys

def send_udp(message, host='localhost', port=9162):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.sendto(message.encode(), (host, port))
        sock.settimeout(3)
        data, _ = sock.recvfrom(4096)
        print(data.decode())
    except socket.timeout:
        print("[ERROR] No response from Zayden-AI")
    except Exception as e:
        print(f"[ERROR] {e}")
    finally:
        sock.close()

if __name__ == "__main__":
    if len(sys.argv) > 1:
        msg = "TALK:" + " ".join(sys.argv[1:])
        send_udp(msg)
    else:
        print("ZAYDEN-AI Chat Client")
        print("Commands: /status, /remember, /recall, /backup, /mutate, /exit")
        print("-" * 40)
        while True:
            try:
                user = input("You: ")
            except EOFError:
                break
            if not user:
                continue
            if user.lower() == '/exit':
                break
            if user.lower() == '/status':
                send_udp("STATUS")
            elif user.lower().startswith('/remember '):
                send_udp("REMEMBER " + user[10:])
            elif user.lower().startswith('/recall '):
                send_udp("RECALL " + user[8:])
            elif user.lower() == '/backup':
                send_udp("BACKUP")
            elif user.lower() == '/mutate':
                send_udp("MUTATE")
            else:
                send_udp("TALK:" + user)
