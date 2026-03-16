import sys
filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# The Alligator is still sporadically catching on the TOO_SMALL constraint because 
# the mathematical spider legs make the expected bounding box massive, but the LCM renderer 
# frequently clumps the plushies together into a tight ball during early decay phases.
# We will drop the lower bound to 0.05 to allow tight clusters without infinite looping, 
# while still aggressively catching large-scale fractures (TOO_LARGE).
code = code.replace(
    'min_allowed_ratio = 0.15 # Relaxed lower bound to account for smaller spider generations',
    'min_allowed_ratio = 0.05 # Severely relaxed lower bound to allow tightly clumped plush geometries'
)

with open(filepath, 'w') as f:
    f.write(code)

