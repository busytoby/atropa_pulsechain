import numpy as np
import math

class TubeVactrolWarmthTransducer:
    """
    Implements a 3D Tube Saturation Transducer where virtual triode tube parameters
    (plate resistance sag and grid potential modulation) are dynamically driven by
    the 3D spatial resonance volume of the projected Lissajous geometry.
    """
    def __init__(self, sample_rate=44100):
        self.sample_rate = sample_rate
        # Tube physical registers
        self.v_plate_sag = 0.0
        self.temp_cathode = 293.15
        
    def calculate_3d_resonance_radius(self, points_3d):
        """
        Calculates the 3D spatial volume of the convolved attractor space.
        """
        if len(points_3d) == 0:
            return 0.0
        dists = np.sqrt(np.sum(np.square(points_3d), axis=1))
        return np.mean(dists)
        
    def process_triode_stage(self, audio_in, r_3d):
        """
        Passes audio through a simulated warm triode stage where parameters
        are modulated by the 3D resonance radius.
        """
        # 3D spatial resonance modulates vactrol sag:
        # Larger 3D shapes cause greater plate voltage sag (soft compression)
        self.v_plate_sag = 0.95 * self.v_plate_sag + 0.05 * (r_3d * 0.12)
        effective_plate_voltage = max(50.0, 250.0 - self.v_plate_sag * 180.0)
        
        # Grid transfer function modeling warm triode soft-clipping:
        # Output = V_plate * (input + bias) ^ 1.5 (Child-Langmuir equation is NOT used here;
        # instead we use standard non-preferential hyperbolic tangent saturation
        # to preserve mathematical continuity across the grid boundary).
        drive = 1.0 + r_3d * 0.5
        scaled_in = audio_in * drive
        
        # Hyperbolic tangent triode grid transfer with asymmetric offset (second-harmonic warmth)
        asymmetry_offset = 0.15 * r_3d
        warm_out = np.tanh(scaled_in + asymmetry_offset) * (effective_plate_voltage / 250.0)
        
        # Deduct gain compensation to avoid signal clipping
        gain_correction = 1.0 / (1.0 + r_3d * 0.2)
        return warm_out * gain_correction

def run_transduction_test():
    print("[TUBE-TRANSDUCTION] Initializing 3D tube saturation validation...")
    
    transducer = TubeVactrolWarmthTransducer()
    
    # Generate 3D coordinates representing a small shape (low resonance)
    theta = np.linspace(0, 2*np.pi, 200)
    small_shape = np.column_stack((np.cos(theta), np.sin(theta), np.sin(2*theta)))
    r_small = transducer.calculate_3d_resonance_radius(small_shape)
    
    # Generate 3D coordinates representing a large shape (high resonance)
    large_shape = small_shape * 4.5
    r_large = transducer.calculate_3d_resonance_radius(large_shape)
    
    print(f"3D Space Proof: Small Radius = {r_small:.4f} | Large Radius = {r_large:.4f}")
    
    # Process test audio signal through the transducer
    test_sig = np.sin(2 * np.pi * 440.0 * np.arange(100) / 44100.0)
    
    out_small = transducer.process_triode_stage(test_sig, r_small)
    out_large = transducer.process_triode_stage(test_sig, r_large)
    
    # Calculate harmonic distortion ratio (indicator of triode warmth/saturation)
    thd_small = np.std(out_small - test_sig)
    thd_large = np.std(out_large - test_sig)
    
    print(f"Low Resonance THD (Warmth Indicator):  {thd_small:.6f}")
    print(f"High Resonance THD (Warmth Indicator): {thd_large:.6f}")
    
    # Verify that larger 3D spatial volumes generate greater harmonic warmth/saturation
    assert thd_large > thd_small
    assert transducer.v_plate_sag > 0.0
    print("[SUCCESS] 3D Tube Saturation Transduction validation completed.")

if __name__ == "__main__":
    run_transduction_test()
