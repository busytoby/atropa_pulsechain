import subprocess
import json
import time
import sys

print("Starting render_vlm_synthesizer.py daemon...")
proc = subprocess.Popen(
    ["python3", "-u", "scripts/render_vlm_synthesizer.py", "--daemon"],
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
    stderr=subprocess.PIPE,
    text=True
)

# Wait for startup output and READY
while True:
    line = proc.stdout.readline()
    if not line:
        break
    print(f"[DAEMON OUT] {line.strip()}")
    if "one-shot mode" in line:
        break

time.sleep(1)

# Send request 1: BZO
req1 = {
    "frame": 777,
    "steps": 2,
    "cfg": 7.5,
    "promptOverride": "bzo bozo currency token digital coin art",
    "address": "0x008fc4bbb1998bfee060f780be7688f0cec66bff"
}
print("\n--- Sending Request 1 ---")
proc.stdin.write(json.dumps(req1) + "\n")
proc.stdin.flush()

while True:
    line = proc.stdout.readline()
    if not line:
        break
    print(f"[DAEMON OUT] {line.strip()}")
    if "__RENDER_COMPLETE__" in line:
        break

# Send request 2: BZO again
print("\n--- Sending Request 2 ---")
proc.stdin.write(json.dumps(req1) + "\n")
proc.stdin.flush()

while True:
    line = proc.stdout.readline()
    if not line:
        break
    print(f"[DAEMON OUT] {line.strip()}")
    if "__RENDER_COMPLETE__" in line:
        break

print("\nTerminating daemon...")
proc.stdin.write("EXIT\n")
proc.stdin.flush()
proc.wait()
print("Test complete.")
