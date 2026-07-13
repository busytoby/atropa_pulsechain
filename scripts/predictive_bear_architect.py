import struct
import json
import os

# Predictive Bear Architect: Eliminates brute force by calculating optimal
# genome parameters based on historical benchmark results.
def predict_and_compile_genome():
    print("=== Auncient Predictive Genome Architect Core ===")
    
    # 1. Read historical benchmark metrics (the ACAB activity ledger)
    results_path = "tsfi2-deepseek/benchmarks/profiler_suite/benchmark_results.json"
    avg_fps = 60.0
    if os.path.exists(results_path):
        try:
            with open(results_path, 'r') as f:
                data = json.load(f)
                avg_fps = data.get("average_fps", 60.0)
                print(f"   - Read ACAB benchmark data. Current Average FPS: {avg_fps:.2f}")
        except Exception as e:
            print(f"   - Warning: Failed to parse benchmark JSON: {e}")

    # 2. Load user preference feedback coordinates from the ACAB ledger
    ledger_path = "assets/bear_evolution_ledger.json"
    upvotes = []
    quarantines = []
    if os.path.exists(ledger_path):
        try:
            with open(ledger_path, 'r') as f:
                history = json.load(f)
                for item in history:
                    genome = item.get("genome")
                    if not genome:
                        continue
                    if item.get("event") == "VOTE_UP_BEAR":
                        upvotes.append(genome)
                    elif item.get("event") == "VOTE_DOWN_QUARANTINE":
                        quarantines.append(genome)
            print(f"   - Read vote ledger. Upvotes: {len(upvotes)}, Bans: {len(quarantines)}")
        except Exception as e:
            print(f"   - Warning: Failed to parse vote ledger: {e}")

    # 3. Calculate target parameters based on preference vectors
    # Default parameters:
    fur_r, fur_g, fur_b = 120, 120, 120
    scale = 120
    fur_length = 110
    twitch = 40
    sickness = 0

    if upvotes:
        # Move parameters towards the average of upvoted bears
        fur_r = int(sum(u['fur_r'] for u in upvotes) / len(upvotes))
        fur_g = int(sum(u['fur_g'] for u in upvotes) / len(upvotes))
        fur_b = int(sum(u['fur_b'] for u in upvotes) / len(upvotes))
        scale = int(sum(u.get('scale', 120) for u in upvotes) / len(upvotes))
        # Visual refinement optimizations
        fur_length = 160 # High quality fur
        twitch = 80
        sickness = 0
        print(f"   - Evolving toward preference target: Color({fur_r},{fur_g},{fur_b}) Scale({scale})")
    else:
        # Static baseline fallback
        fur_r, fur_g, fur_b = 130, 80, 50
        scale = 110

    if quarantines:
        # Move away from banned gray/unhealthy bears
        ban_r = sum(q['fur_r'] for q in quarantines) / len(quarantines)
        if abs(fur_r - ban_r) < 30:
            # Shift hue to avoid quarantine zone
            fur_r = (fur_r + 80) % 256
            fur_g = (fur_g + 50) % 256
            print(f"   - Warning: Evading banned color boundary. Shifted to Color({fur_r},{fur_g},{fur_b})")

    # Static eye parameters
    eye_r, eye_g, eye_b = 0, 255, 0
    light_angle = 180
    breath_freq = 64

    # 3. Direct Genome Synthesis: Write the 12-byte TsfiTeddyDna structure
    # Structure mapping (12 bytes):
    #   fur_r, fur_g, fur_b, eye_r, eye_g, eye_b (6 bytes)
    #   base_sickness, base_scale, base_fur_length, light_angle_deg, breathing_freq, twitch_intensity (6 bytes)
    dna_path = "assets/bear_genome.dna"
    os.makedirs(os.path.dirname(dna_path), exist_ok=True)
    
    dna_bytes = struct.pack(
        '<BBBBBBBBBBBB',
        fur_r, fur_g, fur_b,
        eye_r, eye_g, eye_b,
        sickness, scale, fur_length,
        light_angle, breath_freq, twitch
    )

    with open(dna_path, 'wb') as f:
        f.write(dna_bytes)
        
    print(f"   ✓ Successfully synthesized optimal genome at: {dna_path}")
    print(f"     Parameters: Fur Color({fur_r},{fur_g},{fur_b}), Fur Length: {fur_length/1000.0}m, Scale: {scale/100.0}x")

if __name__ == '__main__':
    predict_and_compile_genome()
