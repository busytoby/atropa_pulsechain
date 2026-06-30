import numpy as np
from PIL import Image, ImageDraw, ImageFont

# MotzkinPrime constant
MotzkinPrime = 953467954114363

LAU_TOKENS = [
    ("mariarahelLau (Ours)", "0xAD4e198623A5E2723e19E4D4a6ECF72B1D19FE4B"),
    ("TestLAU", "0xD07B9f3DF4E9634EbAa0CBF079816925b2C474Ce"),
    ("UT0", "0xd32c39fee49391c7952d1b30b15921b0d3b42e69"),
    ("UT0_2", "0xed343c0f99c89ed7c3c934a88f90261fd6a9a68b"),
    ("UT1", "0x3e10ed242ecb3951151e7a07e0a8f43d4f150c0e")
]

def derive_hypotrochoid_params(address):
    address_int = int(address, 16)
    R_hyp = (address_int % 97) + 20.0
    r_hyp = (address_int % 53) + 10.0
    d_hyp = (address_int % 41) + 5.0
    return R_hyp, r_hyp, d_hyp

def render_lau_panel(name, address, size=256):
    im = Image.new("RGB", (size, size), (11, 15, 25))
    draw = ImageDraw.Draw(im)
    
    R_hyp, r_hyp, d_hyp = derive_hypotrochoid_params(address)
    
    # Generate hypotrochoid coordinates
    pts = []
    for i in range(1000):
        theta = (i * 2.0 * np.pi) / 1000
        h_x = (R_hyp - r_hyp) * np.cos(theta) + d_hyp * np.cos(((R_hyp - r_hyp) / r_hyp) * theta)
        h_y = (R_hyp - r_hyp) * np.sin(theta) - d_hyp * np.sin(((R_hyp - r_hyp) / r_hyp) * theta)
        
        x = int(size / 2 + h_x * 2.5)
        y = int(size / 2 + h_y * 2.5)
        pts.append((x, y))
        
    # Draw coordinates using cyan glow lines
    if len(pts) > 1:
        draw.line(pts, fill=(6, 182, 212), width=2)
        
    # Label the panel
    draw.text((10, 10), name, fill=(255, 255, 255))
    draw.text((10, size - 25), f"R={int(R_hyp)} r={int(r_hyp)} d={int(d_hyp)}", fill=(156, 163, 175))
    return im

def build_composite():
    size = 256
    composite_im = Image.new("RGB", (size * 5, size), (11, 15, 25))
    
    for idx, (name, addr) in enumerate(LAU_TOKENS):
        panel = render_lau_panel(name, addr, size)
        composite_im.paste(panel, (idx * size, 0))
        
    target_path = "/home/mariarahel/.gemini/antigravity-cli/brain/9da2319b-6e2f-4316-8373-cac8f2c06df0/lau_projections_comparison.jpg"
    composite_im.save(target_path, "JPEG")
    print(f"[COMPOSITE] Saved composite image to: {target_path}")

if __name__ == "__main__":
    build_composite()
