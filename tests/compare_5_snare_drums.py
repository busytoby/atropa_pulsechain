import numpy as np
from scripts.generate_eye_of_the_tiger_video import get_lissajous_shape
from tests.test_lau_projections import rasterize_points, calculate_pearson_correlation

def compare_5_snare_drums():
    print("[LAU-TEST] Comparing snare drum projections for 5 distinct LAUs...")
    prime = 953467954114363
    
    # Define 5 distinct LAU configurations
    laus = [
        {"name": "LAU 1 (Base=3)", "Base": 3, "Identity": 11, "Ring": 10},
        {"name": "LAU 2 (Base=5)", "Base": 5, "Identity": 23, "Ring": 22},
        {"name": "LAU 3 (Base=7)", "Base": 7, "Identity": 29, "Ring": 36},
        {"name": "LAU 4 (Base=11)", "Base": 11, "Identity": 31, "Ring": 42},
        {"name": "LAU 5 (Base=13)", "Base": 13, "Identity": 37, "Ring": 50}
    ]
    
    steps = 512
    t = np.linspace(0, 1.0, steps)
    sig_snare = np.random.normal(0.0, 0.5, steps) * np.exp(-t * 8.0)
    
    samplings_snare = {
        "pitch": 400.0, "rms": 0.4, "crest": 3.0, "zcr": 0.6, 
        "brightness": 0.7, "stereo_width": 0.6, "flux": 0.4
    }
    
    # Generate rasterized grids for all 5 LAUs
    grids = []
    for lau in laus:
        state = {
            "Prime": prime, "Base": lau["Base"], "Identity": lau["Identity"], "Ring": lau["Ring"] % prime,
            "Monopole": pow(lau["Base"], 5, prime), "Rod_Dynamo": pow(lau["Base"], 7, prime),
            "Cone_Dynamo": pow(lau["Base"], lau["Identity"], prime), "Manifold": pow(lau["Base"], 5, 8),
            "Element": 8, "Chin": lau["Ring"] % prime
        }
        pts = get_lissajous_shape(state, 0.05, steps, sig_snare, 50.0, samplings_snare, 0, chan_idx=5, is_visual=True)
        grids.append(rasterize_points(pts))
        
    # Calculate pairwise correlation matrix
    print("\nPairwise Spatial Correlation Matrix for Snare Drum:")
    print("      | LAU 1 | LAU 2 | LAU 3 | LAU 4 | LAU 5")
    print("---------------------------------------------")
    for i in range(5):
        row_str = f"LAU {i+1} |"
        for j in range(5):
            corr = calculate_pearson_correlation(grids[i], grids[j])
            row_str += f" {corr:.4f} |"
        print(row_str)

if __name__ == "__main__":
    compare_5_snare_drums()
