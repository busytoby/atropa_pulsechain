import numpy as np
import math
from scripts.generate_eye_of_the_tiger_video import get_lissajous_shape

def rasterize_points(pts, grid_size=64):
    """
    Rasterizes a list of coordinates into a 2D grid matrix.
    """
    grid = np.zeros((grid_size, grid_size))
    if len(pts) == 0:
        return grid
        
    pts_arr = np.array(pts)
    min_val = np.min(pts_arr, axis=0)
    max_val = np.max(pts_arr, axis=0)
    
    # Avoid division by zero
    diff = max_val - min_val
    diff[diff == 0.0] = 1.0
    
    normalized = (pts_arr - min_val) / diff * (grid_size - 1)
    for p in normalized:
        x, y = int(p[0]), int(p[1])
        grid[y, x] = 1.0
        
    return grid

def calculate_pearson_correlation(grid_a, grid_b):
    """
    Calculates the Pearson spatial correlation coefficient between two 2D grids.
    """
    mean_a = np.mean(grid_a)
    mean_b = np.mean(grid_b)
    
    diff_a = grid_a - mean_a
    diff_b = grid_b - mean_b
    
    num = np.sum(diff_a * diff_b)
    den = np.sqrt(np.sum(np.square(diff_a)) * np.sum(np.square(diff_b)))
    
    if den == 0.0:
        return 0.0
    return num / den

def test_lau_projections_similarity():
    print("[LAU-TEST] Starting LAU projection uniqueness unit tests...")
    
    # Setup mock contract states for two different LAUs
    prime = 953467954114363
    
    # LAU Alpha State (Standard setup)
    state_alpha = {
        "Prime": prime, "Base": 3, "Identity": 11, "Ring": 10 % prime,
        "Monopole": pow(3, 5, prime), "Rod_Dynamo": pow(3, 7, prime),
        "Cone_Dynamo": pow(3, 11, prime), "Manifold": pow(3, 5, 8),
        "Element": 8, "Chin": 10 % prime
    }
    
    # LAU Beta State (Distinct base register alignments)
    state_beta = {
        "Prime": prime, "Base": 5, "Identity": 23, "Ring": 22 % prime,
        "Monopole": pow(5, 11, prime), "Rod_Dynamo": pow(5, 17, prime),
        "Cone_Dynamo": pow(5, 23, prime), "Manifold": pow(5, 11, 16),
        "Element": 16, "Chin": 22 % prime
    }
    
    # Mock sound signal input segment
    steps = 512
    t = np.linspace(0, 1.0, steps)
    sig = np.sin(2 * np.pi * 5.0 * t)
    
    samplings = {
        "pitch": 220.0, "rms": 0.5, "crest": 1.0, "zcr": 0.1, 
        "brightness": 0.2, "stereo_width": 0.5, "flux": 0.1
    }
    
    # Generate visual projection paths
    pts_alpha = get_lissajous_shape(state_alpha, 0.0, steps, sig, 100.0, samplings, 0, chan_idx=0, is_visual=True)
    pts_beta = get_lissajous_shape(state_beta, 0.0, steps, sig, 100.0, samplings, 0, chan_idx=0, is_visual=True)
    
    # Rasterize paths
    grid_alpha = rasterize_points(pts_alpha)
    grid_beta = rasterize_points(pts_beta)
    
    # Calculate Pearson spatial correlation
    corr = calculate_pearson_correlation(grid_alpha, grid_beta)
    
    print(f"LAU Alpha vs LAU Beta Spatial Correlation: {corr:.6f}")
    
    # Assert that different LAUs generate distinct visual patterns (low correlation)
    # Correlation of different patterns should be well below 0.85
    assert corr < 0.80, f"Error: LAU projections are too similar! Correlation = {corr:.6f}"
    
    # Assert self-correlation is perfect 1.0
    self_corr = calculate_pearson_correlation(grid_alpha, grid_alpha)
    assert np.allclose(self_corr, 1.0)
    
    print("[SUCCESS] LAU master channel projection uniqueness unit test passed.")

    # ----------------------------------------------------
    # Snare Drum channel test (chan_idx=5)
    # ----------------------------------------------------
    print("[LAU-TEST] Running snare drum projection uniqueness test (chan_idx=5)...")
    # Generate snare-like noise burst transient
    np.random.seed(42)
    sig_snare = np.random.normal(0.0, 0.5, steps) * np.exp(-t * 8.0)
    
    samplings_snare = {
        "pitch": 400.0, "rms": 0.4, "crest": 3.0, "zcr": 0.6, 
        "brightness": 0.7, "stereo_width": 0.6, "flux": 0.4
    }
    
    pts_alpha_snare = get_lissajous_shape(state_alpha, 0.05, steps, sig_snare, 50.0, samplings_snare, 0, chan_idx=5, is_visual=True)
    pts_beta_snare = get_lissajous_shape(state_beta, 0.05, steps, sig_snare, 50.0, samplings_snare, 0, chan_idx=5, is_visual=True)
    
    grid_alpha_snare = rasterize_points(pts_alpha_snare)
    grid_beta_snare = rasterize_points(pts_beta_snare)
    
    corr_snare = calculate_pearson_correlation(grid_alpha_snare, grid_beta_snare)
    print(f"LAU Snare Drum Spatial Correlation: {corr_snare:.6f}")
    
    assert corr_snare < 0.80, f"Error: LAU snare drum projections are too similar! Correlation = {corr_snare:.6f}"
    print("[SUCCESS] LAU snare drum projection uniqueness unit test passed.")

if __name__ == "__main__":
    test_lau_projections_similarity()
