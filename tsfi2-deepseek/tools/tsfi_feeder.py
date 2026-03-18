import sys
import time
import os

if len(sys.argv) < 2:
    sys.exit(1)

source = sys.argv[1]
fps = 30
frame_time = 1.0 / fps

# Read the initial image to avoid empty pipe
while not os.path.exists(source):
    time.sleep(0.1)

current_data = b""
with open(source, 'rb') as f:
    current_data = f.read()

while True:
    start = time.time()
    
    # Try to update current_data if file exists and is readable
    try:
        if os.path.exists(source):
            with open(source, 'rb') as f:
                new_data = f.read()
                # Check for JPEG header to ensure complete write
                if len(new_data) > 0 and new_data[0:2] == b'\xff\xd8':
                    current_data = new_data
    except Exception as e:
        # Ignore read errors (e.g. file is being written to)
        pass

    if current_data:
        try:
            sys.stdout.buffer.write(current_data)
            sys.stdout.buffer.flush()
        except BrokenPipeError:
            sys.exit(0)
    
    elapsed = time.time() - start
    wait = frame_time - elapsed
    if wait > 0:
        time.sleep(wait)
