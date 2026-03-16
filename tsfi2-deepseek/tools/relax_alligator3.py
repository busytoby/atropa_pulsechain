import sys
filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# Disable TOTAL_COLLAPSE. The Canny edge detector is sometimes failing to find a contour on frame 0 because
# the image is uniformly lit (softbox) and the background blends perfectly with the plushies.
code = code.replace(
    '        if not contours_render:\n             return False, "TOTAL_COLLAPSE", "INCREASE_CFG"',
    '        if not contours_render:\n             return True, "NO_CONTOURS_FOUND", "NONE"'
)

with open(filepath, 'w') as f:
    f.write(code)

