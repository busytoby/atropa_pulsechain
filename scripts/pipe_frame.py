import sys
import os
import struct

def main():
    frame_path = "frontend/latest_frame.jpg"
    if not os.path.exists(frame_path):
        sys.stderr.write(f"Error: {frame_path} not found\n")
        sys.exit(1)
        
    with open(frame_path, "rb") as f:
        jpeg_data = f.read()
        
    jpeg_len = len(jpeg_data)
    # Header format: [uint32 jpeg_len][int32 x][int32 y][int32 w][int32 h]
    # We want to draw it full screen, so we can pass destination bounds of the main frame
    # g_dest_x, g_dest_y, g_dest_w, g_dest_h. Let's pass 0, 0, 1024, 768.
    header = struct.pack("<Iiiii", jpeg_len, 0, 0, 1024, 768)
    
    # Write to stdout
    sys.stdout.buffer.write(header)
    sys.stdout.buffer.write(jpeg_data)
    sys.stdout.buffer.flush()
    sys.stderr.write(f"Piped frame of size {jpeg_len} bytes to presenter stdin.\n")

if __name__ == "__main__":
    main()
