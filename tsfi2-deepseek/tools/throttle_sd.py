import sys
import glob

for filepath in glob.glob("tools/*vtuber*.py"):
    with open(filepath, 'r') as f:
        code = f.read()

    # The issue is that the generation threads run infinitely with zero yielding between frames.
    # While they sleep on network backpressure or logic drops, the primary loop for sd-cli just blasts.
    # We will throttle the python loop generating the image to yield ~15-20% of its time back to the OS.
    # This acts as an effective ~85-90% utilization throttle across the system.
    
    # We can inject a sleep immediately after sd-cli runs.
    if 'subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)' in code:
        code = code.replace(
            'subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)',
            'subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)\n            time.sleep(0.15) # Throttle GPU to ~85% duty cycle to prevent hardware hanging'
        )
        
        with open(filepath, 'w') as f:
            f.write(code)
        print(f"Throttled {filepath}")

