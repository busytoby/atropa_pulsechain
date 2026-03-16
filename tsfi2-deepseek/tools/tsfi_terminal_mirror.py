import os
import mmap
import struct

def render_fb_to_terminal():
    fb_path = "/dev/shm/tsfi_wayland_fb"
    w = 800
    h = 600
    sz = w * h * 4
    
    if not os.path.exists(fb_path):
        print(f"[FRACTURE] Framebuffer offline.")
        return
        
    fd = os.open(fb_path, os.O_RDONLY)
    fb_map = mmap.mmap(fd, sz, mmap.MAP_SHARED, mmap.PROT_READ)
    
    # We will sample the 800x600 buffer down to a 80x40 terminal grid
    sample_w = 80
    sample_h = 40
    step_x = w // sample_w
    step_y = h // sample_h
    
    print("=== TSFi Wayland Terminal Mirror ===")
    
    for ty in range(sample_h):
        line = ""
        for tx in range(sample_w):
            px_y = ty * step_y
            px_x = tx * step_x
            idx = (px_y * w + px_x) * 4
            
            pixel = struct.unpack('=I', fb_map[idx:idx+4])[0]
            color = pixel & 0x00FFFFFF
            
            if color == 0:
                line += " " # Void
            elif color == 0x00FF00:
                line += "\033[92m#\033[0m" # Kr0wZ Green
            elif color == 0x00AAFF:
                line += "\033[96m#\033[0m" # Pulse Blue
            elif color == 0xFF0000:
                line += "\033[91m#\033[0m" # Fracture Red
            else:
                line += "\033[97m#\033[0m" # Other
                
        print(line)
        
    fb_map.close()
    os.close(fd)

if __name__ == "__main__":
    render_fb_to_terminal()
