#!/usr/bin/env python3
"""Proxy relay with auth injection - handles multiple Chrome connections properly."""
import sys, socket, select, base64, threading, time

def handle_client(client_sock, upstream_host, upstream_port, proxy_user, proxy_pass):
    upstream = None
    try:
        upstream = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        upstream.settimeout(10)
        upstream.connect((upstream_host, int(upstream_port)))

        # Read first request from client
        client_sock.settimeout(5)
        first_data = client_sock.recv(65536)
        if not first_data:
            return

        # Inject auth if credentials provided
        if proxy_user and proxy_pass:
            auth = base64.b64encode(f"{proxy_user}:{proxy_pass}".encode()).decode()
            auth_line = f"Proxy-Authorization: Basic {auth}\r\n".encode()
            idx = first_data.index(b"\r\n")
            first_data = first_data[:idx+2] + auth_line + first_data[idx+2:]

        upstream.sendall(first_data)

        # Bidirectional forwarding with no timeouts
        client_sock.settimeout(None)
        upstream.settimeout(None)

        rlist = [client_sock, upstream]
        while True:
            readable, _, exceptional = select.select(rlist, [], rlist, 30)
            if exceptional:
                break
            if not readable:
                break
            for sock in readable:
                try:
                    data = sock.recv(65536)
                except:
                    return
                if not data:
                    return
                target = upstream if sock is client_sock else client_sock
                try:
                    target.sendall(data)
                except:
                    return
    except Exception as e:
        import traceback; traceback.print_exc()
    finally:
        for s in (client_sock, upstream):
            try: s.close()
            except: pass

def main():
    local_port = int(sys.argv[1])
    upstream_host = sys.argv[2]
    upstream_port = int(sys.argv[3])
    proxy_user = sys.argv[4] if len(sys.argv) > 4 else ""
    proxy_pass = sys.argv[5] if len(sys.argv) > 5 else ""

    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("127.0.0.1", local_port))
    server.listen(50)

    print(f"[Proxy Relay] Listening on 127.0.0.1:{local_port} -> {upstream_host}:{upstream_port}", flush=True)

    workers = []
    try:
        while True:
            client, addr = server.accept()
            t = threading.Thread(
                target=handle_client,
                args=(client, upstream_host, upstream_port, proxy_user, proxy_pass),
                daemon=True
            )
            t.start()
            workers.append(t)
            # Clean up finished threads occasionally
            if len(workers) > 50:
                workers = [w for w in workers if w.is_alive()]
    except KeyboardInterrupt:
        pass
    finally:
        server.close()

if __name__ == "__main__":
    main()
