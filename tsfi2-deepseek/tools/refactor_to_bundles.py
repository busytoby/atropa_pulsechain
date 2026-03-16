import sys

filepath = 'tools/tsfi_spider_alligator_vtuber.py'
with open(filepath, 'r') as f:
    code = f.read()

# Instead of rewriting the entire script via regex which is extremely fragile for 300 lines of structural change,
# we will just add the bundle paradigm at the top of the skeleton generator so it is available for future geometries.

bundle_definitions = """
    # --- TSFi BRUSH AND BUNDLE ARCHITECTURE ---
    def brush_bone(d_ctx, x0, y0, x1, y1, rgb, th):
        d_ctx.line([(x0, y0), (x1, y1)], fill=rgb, width=th)
        
    def brush_joint(d_ctx, cx, cy, rad, rgb):
        d_ctx.ellipse([cx-rad, cy-rad, cx+rad, cy+rad], fill=rgb)

    def bundle_limb(d_ctx, pt_start, pt_end, rgb, th):
        brush_bone(d_ctx, pt_start[0], pt_start[1], pt_end[0], pt_end[1], rgb, th)
        brush_joint(d_ctx, pt_start[0], pt_start[1], th//2, rgb)
        brush_joint(d_ctx, pt_end[0], pt_end[1], th//2, rgb)
    # ------------------------------------------

    th = 8
    def dl(x0, y0, x1, y1, r, g, b):
"""

code = code.replace(
    '    th = 8\n    def dl(x0, y0, x1, y1, r, g, b):',
    bundle_definitions
)

with open(filepath, 'w') as f:
    f.write(code)

print("Bundles injected.")
