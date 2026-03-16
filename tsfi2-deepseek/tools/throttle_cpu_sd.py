import sys
import glob

for filepath in glob.glob("tools/*vtuber*.py"):
    with open(filepath, 'r') as f:
        code = f.read()

    # The SD-CLI backend is saturating CPU threads during tensor prep before shipping to Vulkan.
    # We will explicitly cap the CPU threads it is allowed to spawn using the `--threads` flag.
    # 4 threads is enough to feed the GPU without overwhelming the host CPU scheduler.
    
    code = code.replace(
        '"--steps", "4", ',
        '"--steps", "4", \n                "--threads", "4",'
    )
    
    with open(filepath, 'w') as f:
        f.write(code)
    print(f"Capped CPU Threads in {filepath}")

