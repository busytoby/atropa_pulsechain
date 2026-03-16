import sys
import glob

for filepath in glob.glob("tools/*vtuber*.py"):
    with open(filepath, 'r') as f:
        code = f.read()

    # The 0.15s sleep wasn't enough to drop the GPU load below 100% since `sd-cli` process startup/teardown
    # is fast but the Vulkan memory queue still stacks up heavily. 
    # We will expand the throttle significantly to 0.5s per frame. 
    # This will drop generation speed to ~2 FPS natively (which FFmpeg smoothly duplicates to 30fps for YouTube),
    # deeply reducing GPU compute pressure and system hang.
    
    code = code.replace(
        'time.sleep(0.15) # Throttle GPU to ~85% duty cycle to prevent hardware hanging',
        'time.sleep(0.5) # Aggressive throttle to ~60% duty cycle to prevent full system stuttering'
    )
    
    with open(filepath, 'w') as f:
        f.write(code)
    print(f"Aggressively Throttled {filepath}")

