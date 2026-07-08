import sys
import socket
import threading
import select
import base64

def handle_client(client_sock, upstream_host, upstream_port, proxy_user=None, proxy_pass=None):
    upstream = None
    try:
        upstream = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        upstream.settimeout(10)
        upstream.connect((upstream_host, int(upstream_port)))
        upstream.settimeout(None)
        client_sock.settimeout(5)

        first_data = client_sock.recv(65536)
        if not first_data:
            return

        if proxy_user and proxy_pass and first_data.startswith(b"CONNECT"):
            # Build Proxy-Authorization header and inject into CONNECT request
            auth = base64.b64encode(f"{proxy_user}:{proxy_pass}".encode()).decode()
            auth_header = f"Proxy-Authorization: Basic {auth}\r\n".encode()
            
            # CONNECT request format:
            #   CONNECT host:port HTTP/1.1\r\n
            #   Header1: val1\r\n
            #   \r\n
            # Insert auth_header right after the first line
            idx = first_data.index(b"\r\n")
            modified = first_data[:idx+2] + auth_header + first_data[idx+2:]
            
            upstream.sendall(modified)
            print(f"[Relay] CONNECT with auth -> {upstream_host}:{upstream_port}")
        else:
            upstream.sendall(first_data)

        client_sock.settimeout(None)

        sockets = [client_sock, upstream]
        while True:
            readable, _, _ = select.select(sockets, [], [], 30)
            if not readable:
                break
            for sock in readable:
                data = sock.recv(65536)
                if not data:
                    return
                target = upstream if sock is client_sock else client_sock
                try:
                    target.sendall(data)
                except:
                    return
    except ConnectionResetError:
        pass
    except Exception as e:
        print(f"[Relay Error] {e}")
    finally:
        try:
            client_sock.close()
        except:
            pass
        if upstream:
            try:
                upstream.close()
            except:
                pass

def main():
    if len(sys.argv) < 3:
        print("Usage: proxy_relay.py <local_port> <upstream_host> <upstream_port> [proxy_user] [proxy_pass]")
        sys.exit(1)
    
    local_port = int(sys.argv[1])
    upstream_host = sys.argv[2]
    upstream_port = int(sys.argv[3])
    proxy_user = sys.argv[4] if len(sys.argv) > 4 else None
    proxy_pass = sys.argv[5] if len(sys.argv) > 5 else None
    
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.bind(("127.0.0.1", local_port))
    server.listen(50)
    server.settimeout(1)
    
    print(f"[Proxy Relay] Listening on 127.0.0.1:{local_port} -> {upstream_host}:{upstream_port}")
    if proxy_user:
        print(f"[Proxy Relay] Auth: {proxy_user}:****")
    
    try:
        while True:
            try:
                client, addr = server.accept()
                t = threading.Thread(target=handle_client, args=(client, upstream_host, upstream_port, proxy_user, proxy_pass))
                t.daemon = True
                t.start()
            except socket.timeout:
                continue
    except KeyboardInterrupt:
        print("\n[Proxy Relay] Shutting down.")
    finally:
        server.close()

if __name__ == "__main__":
    main()
