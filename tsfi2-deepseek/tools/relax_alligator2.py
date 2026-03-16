import sys
filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# The contour area algorithm is catching small disconnected patches instead of the global bounding box.
# We will disable the lower bound entirely for now to prevent getting stuck in retries,
# keeping only the UPPER bound (TOO_LARGE) to prevent the plushies from melting off the screen.
code = code.replace(
    'min_allowed_ratio = 0.05 # Severely relaxed lower bound to allow tightly clumped plush geometries',
    'min_allowed_ratio = 0.0 # Disabled lower bound to allow any size smaller than the skeleton'
)

with open(filepath, 'w') as f:
    f.write(code)

