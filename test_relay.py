"""Test proxy_relay.py independently - verifies IPRoyal proxy + auth works."""
import subprocess, sys, time, urllib.request, socket

RELAY_PORT = 55432
PROXY_HOST = "140.228.19.186"
PROXY_PORT = 12323
PROXY_USER = "14ad39895147d"
PROXY_PASS = "031109feaa"
RELAY_SCRIPT = r"E:\Documents\untitled3\proxy_relay.py"

print("=" * 55)
print("Proxy Relay Test - Standalone")
print("=" * 55)

# 1. Start relay
print(f"\n[1] Starting relay on 127.0.0.1:{RELAY_PORT} -> {PROXY_HOST}:{PROXY_PORT}")
relay = subprocess.Popen(
    [sys.executable, RELAY_SCRIPT, str(RELAY_PORT), PROXY_HOST, str(PROXY_PORT), PROXY_USER, PROXY_PASS],
    stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True
)
time.sleep(1.5)

if relay.poll() is not None:
    print("FAIL: Relay exited immediately. Output:")
    print(relay.stdout.read())
    sys.exit(1)

print("   Relay PID:", relay.pid)

# 2. Test direct IP (no proxy)
print("\n[2] Testing DIRECT connection (no proxy)...")
try:
    req = urllib.request.Request("http://httpbin.org/ip", headers={"User-Agent": "Mozilla/5.0"})
    resp = urllib.request.urlopen(req, timeout=10)
    print("   Direct IP response:", resp.read().decode()[:200])
except Exception as e:
    print("   Direct test failed:", e)

# 3. Test through relay
print(f"\n[3] Testing THROUGH relay (127.0.0.1:{RELAY_PORT})...")
proxy_handler = urllib.request.ProxyHandler({"http": f"http://127.0.0.1:{RELAY_PORT}"})
opener = urllib.request.build_opener(proxy_handler)
try:
    req = urllib.request.Request("http://httpbin.org/ip", headers={"User-Agent": "Mozilla/5.0"})
    resp = opener.open(req, timeout=15)
    body = resp.read().decode()
    print("   Relay IP response:", body[:200])
except Exception as e:
    print("   Relay test failed:", e)

# 4. Test HTTPS through relay
print(f"\n[4] Testing HTTPS through relay...")
proxy_handler = urllib.request.ProxyHandler({"https": f"http://127.0.0.1:{RELAY_PORT}"})
opener = urllib.request.build_opener(proxy_handler)
try:
    req = urllib.request.Request("https://httpbin.org/ip", headers={"User-Agent": "Mozilla/5.0"})
    resp = opener.open(req, timeout=15)
    body = resp.read().decode()
    print("   HTTPS IP response:", body[:200])
except Exception as e:
    print("   HTTPS test failed:", e)

# 5. Cleanup
print("\n[5] Stopping relay...")
relay.terminate()
try:
    relay.wait(timeout=3)
except subprocess.TimeoutExpired:
    relay.kill()
print("   Relay stopped.")

print("\n" + "=" * 55)
print("Test complete.")
print("=" * 55)
