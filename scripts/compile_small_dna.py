import struct
import sys
import os
from opencv_teddy_evaluator import tokenize_query

def compile_small_dna(query_str):
    print(f"=== Compiling 12-Byte Procedural Bear Genome ===")
    targets = tokenize_query(query_str)
    
    # Pack parameters:
    # uint8_t fur_r, fur_g, fur_b
    # uint8_t eye_r, eye_g, eye_b
    # uint8_t base_sickness
    # uint8_t base_scale
    # uint8_t base_fur_length
    # uint8_t light_angle_deg
    # uint8_t breathing_freq
    # uint8_t twitch_intensity
    
    fur_r, fur_g, fur_b = targets["fur_rgb"]
    eye_r, eye_g, eye_b = targets["eyes_rgb"]
    base_sickness = int(targets["sickness"] * 100) # 0-100
    base_scale = 100 # 1.0f base scale
    
    # 0.08f -> 80
    base_fur_length = 80
    if base_sickness > 50:
        base_fur_length = 140 # Matched longer sick fur
        
    light_angle_deg = int(135 / 360.0 * 255.0)
    breathing_freq = 128
    twitch_intensity = 128
    
    dna_data = struct.pack('=BBBBBBBBBBBB',
                           fur_r, fur_g, fur_b,
                           eye_r, eye_g, eye_b,
                           base_sickness,
                           base_scale,
                           base_fur_length,
                           light_angle_deg,
                           breathing_freq,
                           twitch_intensity)
                           
    out_dir = "tsfi2-deepseek/assets"
    os.makedirs(out_dir, exist_ok=True)
    out_path = os.path.join(out_dir, "bear_genome.dna")
    
    with open(out_path, "wb") as f:
        f.write(dna_data)
        
    print(f"[SUCCESS] 12-byte genome compiled successfully: {out_path}")
    print(f"          Fur: RGB({fur_r},{fur_g},{fur_b}) | Eyes: RGB({eye_r},{eye_g},{eye_b}) | Sickness: {base_sickness}%")

if __name__ == "__main__":
    query = sys.argv[1] if len(sys.argv) > 1 else "A golden teddy bear with green eyes"
    # Ensure scripts directory is in path
    sys.path.append(os.path.dirname(os.path.abspath(__file__)))
    compile_small_dna(query)
