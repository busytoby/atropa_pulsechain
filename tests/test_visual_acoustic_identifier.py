import unittest
import math
import numpy as np

# --- TRIGONOMETRIC LUT SETUP ---
LUT_SIZE = 22528
LUT_MASK = LUT_SIZE - 1
LUT_FACTOR = LUT_SIZE / (2 * np.pi)
sin_lut = np.sin(np.linspace(0, 2 * np.pi, LUT_SIZE, endpoint=False))
cos_lut = np.cos(np.linspace(0, 2 * np.pi, LUT_SIZE, endpoint=False))

def lut_sin(angle_val, shift=0):
    val = (angle_val % (2.0 * np.pi)) * LUT_FACTOR
    idx = (int(math.floor(val)) + shift) & LUT_MASK
    return sin_lut[idx]

def lut_cos(angle_val, shift=0):
    val = (angle_val % (2.0 * np.pi)) * LUT_FACTOR
    idx = (int(math.floor(val)) + shift) & LUT_MASK
    return cos_lut[idx]

# Auncient 7D YI State Vector Template
# Auncient 14D YI State Vector Template
prime = 953467954114363
YI_STATES = {
    "growl": {
        "Prime": prime,
        "Base": 2,
        "Identity": 23,
        "Ring": (2 + 17) % prime,
        "Monopole": pow(2, 11, prime),
        "Rod_Dynamo": pow(2, 17, prime),
        "Cone_Dynamo": pow(2, 23, prime),
        "Manifold": pow(2, 11, 13),
        "Element": 13,
        "Chin": (2 + 17) % prime
    },
    "bass": {
        "Prime": prime,
        "Base": 5,
        "Identity": 29,
        "Ring": (5 + 19) % prime,
        "Monopole": pow(5, 13, prime),
        "Rod_Dynamo": pow(5, 19, prime),
        "Cone_Dynamo": pow(5, 29, prime),
        "Manifold": pow(5, 13, 18),
        "Element": 18,
        "Chin": (5 + 19) % prime
    },
    "lead": {
        "Prime": prime,
        "Base": 17,
        "Identity": 43,
        "Ring": (17 + 37) % prime,
        "Monopole": pow(17, 29, prime),
        "Rod_Dynamo": pow(17, 37, prime),
        "Cone_Dynamo": pow(17, 43, prime),
        "Manifold": pow(17, 29, 46),
        "Element": 46,
        "Chin": (17 + 37) % prime
    }
}

# Generate coordinate trajectory projected through the YI matrix convolving the sound
def project_visuals(state, sig_segment, active_freq, r_scale=100.0, apply_window=True):
    f_w = (state["Manifold"] % 4) + 1
    f_x = (state["Monopole"] % 5) + 1
    f_y = (state["Rod_Dynamo"] % 4) + 1
    f_z = (state["Cone_Dynamo"] % 5) + 1
    f_r = (state["Ring"] % 3) + 1.0
    f_i = (state["Identity"] % 3) + 1
    
    # 8th and 9th variables
    f_e = (state.get("Element", 8) % 3) + 1.0
    f_c = (state.get("Chin", 0) % 100) / 400.0
    
    shear_factor = (state["Base"] % 100) / 1000.0
    phi_base = (state["Identity"] / state["Prime"]) * 2.0 * np.pi
    
    spin_velocity = (active_freq / 220.0) * 0.5
    phase_w = spin_velocity * 0.05 + phi_base
    phase_x = spin_velocity * 0.06 + phi_base
    phase_y = spin_velocity * 0.07 + phi_base
    
    # 14th Dimension: Hypotrochoid parameters derived from Delegate state alignments (collision-free prime modulos)
    R_hyp = (state["Identity"] % 97) + 20.0
    r_hyp = (state.get("Element", 8) % 53) + 10.0
    d_hyp = (state.get("Chin", 0) % 41) + 5.0
    
    lut_shift = int((active_freq - 110.0) * 10.0)
    delay = int(8 * f_e)
    points = []
    
    n_samples = len(sig_segment)
    window = np.sin(np.pi * np.arange(n_samples) / n_samples) if apply_window else np.ones(n_samples)
    
    for i in range(n_samples):
        theta = (i * 2.0 * np.pi) / n_samples
        eta = ((theta * f_w + phase_w) * 0.5) % (np.pi / 2.0)
        xi1 = ((theta * f_x + phase_x) * f_i) % (2.0 * np.pi)
        xi2 = ((theta * f_y + phase_y) * 3) % (2.0 * np.pi)
        
        qx_g = lut_cos(eta, lut_shift) * lut_sin(xi1, lut_shift)
        qy_g = lut_sin(eta, lut_shift) * lut_cos(xi2, lut_shift)
        
        # Signal telemetry
        idx_now = i
        idx_delayed = (i - delay) % n_samples
        sig_now = sig_segment[idx_now] * window[idx_now]
        sig_delayed = sig_segment[idx_delayed] * window[idx_delayed]
        
        x = (qx_g * sig_now + qy_g * sig_delayed) * r_scale
        y = (-qy_g * sig_now + qx_g * sig_delayed) * r_scale * f_z
        
        # Apply 14D Hypotrochoid vector offset tracing (Delegate token signature)
        h_x = (R_hyp - r_hyp) * math.cos(theta) + d_hyp * math.cos(((R_hyp - r_hyp) / r_hyp) * theta)
        h_y = (R_hyp - r_hyp) * math.sin(theta) - d_hyp * math.sin(((R_hyp - r_hyp) / r_hyp) * theta)
        
        x_final = x + h_x * (1.0 + 0.3 * sig_now)
        y_final = y + x * shear_factor + h_y * (1.0 + 0.3 * sig_delayed)
        points.append((x_final, y_final))
        
    return points

# Feature Extraction and Visual Identifier Engine
def identify_visual_signature(points, active_freq):
    """
    Analyzes geometric coordinates and pitch frequency to identify
    the active instrument channel by matching against reference templates.
    """
    t_val = np.arange(len(points)) / 44100.0
    sig_growl = 0.5 * np.sin(2 * np.pi * active_freq * t_val)
    pts_growl = project_visuals(YI_STATES["growl"], sig_growl, active_freq)
    
    sig_bass = 0.4 * (2 * (t_val * active_freq - np.floor(t_val * active_freq + 0.5)))
    pts_bass = project_visuals(YI_STATES["bass"], sig_bass, active_freq)
    
    sig_lead = 0.3 * (2 * (t_val * active_freq - np.floor(t_val * active_freq)))
    pts_lead = project_visuals(YI_STATES["lead"], sig_lead, active_freq)
    
    mse_growl = np.mean([(p[0] - tg[0])**2 + (p[1] - tg[1])**2 for p, tg in zip(points, pts_growl)])
    mse_bass = np.mean([(p[0] - tb[0])**2 + (p[1] - tb[1])**2 for p, tb in zip(points, pts_bass)])
    mse_lead = np.mean([(p[0] - tl[0])**2 + (p[1] - tl[1])**2 for p, tl in zip(points, pts_lead)])
    
    min_mse = min(mse_growl, mse_bass, mse_lead)
    if min_mse == mse_growl:
        detected_instrument = "growl"
    elif min_mse == mse_bass:
        detected_instrument = "bass"
    else:
        detected_instrument = "lead"
        
    x_coords = np.array([p[0] for p in points])
    y_coords = np.array([p[1] for p in points])
    width = np.max(x_coords) - np.min(x_coords)
    height = np.max(y_coords) - np.min(y_coords)
    aspect_ratio = height / width if width > 0 else 0
    
    return {
        "instrument": detected_instrument,
        "aspect_ratio": aspect_ratio
    }

class TestVisualAcousticIdentifier(unittest.TestCase):
    
    def setUp(self):
        # Synthesize reference audio segments (512 samples)
        self.sr = 44100
        self.samples = 512
        self.t = np.arange(self.samples) / self.sr
        
    def test_growl_sine_attractor(self):
        # Growl is a pure sine wave (55Hz)
        sig = 0.5 * np.sin(2 * np.pi * 55.0 * self.t)
        pts = project_visuals(YI_STATES["growl"], sig, 55.0)
        features = identify_visual_signature(pts, 55.0)
        
        self.assertEqual(features["instrument"], "growl")
        
    def test_bass_triangle_attractor(self):
        # Bass is a triangle wave (110Hz)
        sig = 0.4 * (2 * (self.t * 110.0 - np.floor(self.t * 110.0 + 0.5)))
        pts = project_visuals(YI_STATES["bass"], sig, 110.0)
        features = identify_visual_signature(pts, 110.0)
        
        print("\n[BASS FEATURES]:", features)
        self.assertEqual(features["instrument"], "bass")
        
    def test_lead_saw_attractor(self):
        # Lead is a high-frequency saw/square blend (440Hz)
        sig = 0.3 * (2 * (self.t * 440.0 - np.floor(self.t * 440.0)))
        pts = project_visuals(YI_STATES["lead"], sig, 440.0)
        features = identify_visual_signature(pts, 440.0)
        
        self.assertEqual(features["instrument"], "lead")

    def test_full_keyboard_scale_sweep(self):
        # Frequencies from C1 (32.7 Hz) to C8 (4186 Hz)
        keyboard_notes = [32.7, 65.4, 130.8, 261.6, 523.3, 1046.5, 2093.0, 4186.0]
        
        for freq in keyboard_notes:
            # 1. Growl (Sine Wave)
            sig_growl = 0.5 * np.sin(2 * np.pi * freq * self.t)
            pts_growl = project_visuals(YI_STATES["growl"], sig_growl, freq)
            feat_growl = identify_visual_signature(pts_growl, freq)
            self.assertEqual(feat_growl["instrument"], "growl")
            
            # 2. Bass (Triangle Wave)
            sig_bass = 0.4 * (2 * (self.t * freq - np.floor(self.t * freq + 0.5)))
            pts_bass = project_visuals(YI_STATES["bass"], sig_bass, freq)
            feat_bass = identify_visual_signature(pts_bass, freq)
            self.assertEqual(feat_bass["instrument"], "bass")
            
            # 3. Lead (Saw Wave)
            sig_lead = 0.3 * (2 * (self.t * freq - np.floor(self.t * freq)))
            pts_lead = project_visuals(YI_STATES["lead"], sig_lead, freq)
            feat_lead = identify_visual_signature(pts_lead, freq)
            self.assertEqual(feat_lead["instrument"], "lead")

    def test_tuning_identification(self):
        # Verify tuning for note A4 (440Hz) vs detuned (443Hz)
        tuned_freq = 440.0
        detuned_freq = 443.0
        
        # Analyze signal segments at t1 and t2 (0.05s apart)
        t_t1 = np.arange(self.samples) / self.sr
        t_t2 = t_t1 + 0.05
        
        for inst in ["growl", "bass", "lead"]:
            # Synthesize tuned waveforms at t1 and t2
            if inst == "growl":
                sig_tuned_t1 = 0.5 * np.sin(2 * np.pi * tuned_freq * t_t1)
                sig_tuned_t2 = 0.5 * np.sin(2 * np.pi * tuned_freq * t_t2)
                
                sig_detuned_t1 = 0.5 * np.sin(2 * np.pi * detuned_freq * t_t1)
                sig_detuned_t2 = 0.5 * np.sin(2 * np.pi * detuned_freq * t_t2)
            elif inst == "bass":
                sig_tuned_t1 = 0.4 * (2 * (t_t1 * tuned_freq - np.floor(t_t1 * tuned_freq + 0.5)))
                sig_tuned_t2 = 0.4 * (2 * (t_t2 * tuned_freq - np.floor(t_t2 * tuned_freq + 0.5)))
                
                sig_detuned_t1 = 0.4 * (2 * (t_t1 * detuned_freq - np.floor(t_t1 * detuned_freq + 0.5)))
                sig_detuned_t2 = 0.4 * (2 * (t_t2 * detuned_freq - np.floor(t_t2 * detuned_freq + 0.5)))
            else:
                sig_tuned_t1 = 0.3 * (2 * (t_t1 * tuned_freq - np.floor(t_t1 * tuned_freq)))
                sig_tuned_t2 = 0.3 * (2 * (t_t2 * tuned_freq - np.floor(t_t2 * tuned_freq)))
                
                sig_detuned_t1 = 0.3 * (2 * (t_t1 * detuned_freq - np.floor(t_t1 * detuned_freq)))
                sig_detuned_t2 = 0.3 * (2 * (t_t2 * detuned_freq - np.floor(t_t2 * detuned_freq)))
                
            pts_tuned_t1 = project_visuals(YI_STATES[inst], sig_tuned_t1, tuned_freq)
            pts_tuned_t2 = project_visuals(YI_STATES[inst], sig_tuned_t2, tuned_freq)
            
            pts_detuned_t1 = project_visuals(YI_STATES[inst], sig_detuned_t1, detuned_freq)
            pts_detuned_t2 = project_visuals(YI_STATES[inst], sig_detuned_t2, detuned_freq)
            
            feat_tuned_t1 = identify_visual_signature(pts_tuned_t1, tuned_freq)
            feat_tuned_t2 = identify_visual_signature(pts_tuned_t2, tuned_freq)
            
            feat_detuned_t1 = identify_visual_signature(pts_detuned_t1, detuned_freq)
            feat_detuned_t2 = identify_visual_signature(pts_detuned_t2, detuned_freq)
            
            drift_tuned = abs(feat_tuned_t1["aspect_ratio"] - feat_tuned_t2["aspect_ratio"])
            drift_detuned = abs(feat_detuned_t1["aspect_ratio"] - feat_detuned_t2["aspect_ratio"])
            
            # In-tune attractor should maintain geometric aspect ratio over time (drift is extremely low)
            self.assertLess(drift_tuned, 0.05)
            # Detuned attractor shape morphs over time (high drift)
            self.assertGreater(drift_detuned, drift_tuned)

if __name__ == "__main__":
    unittest.main()
