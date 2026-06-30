import numpy as np
from PIL import Image, ImageDraw
import math

class OpticalVerificationTransducer:
    """
    Monitors and decodes convolved output projection states in the visual domain.
    Correlates the master attractor shape against known pre-master templates to 
    visually resolve instrument vectors without querying the audio signals.
    """
    def __init__(self, size=256):
        self.size = size
        
    def rasterize_coordinates(self, points, scale=2.5):
        im = Image.new("L", (self.size, self.size), 0)
        draw = ImageDraw.Draw(im)
        # Shift and scale coordinates to fit target canvas
        pixel_points = []
        for x, y in points:
            px = int(self.size / 2 + x * scale)
            py = int(self.size / 2 + y * scale)
            pixel_points.append((px, py))
            
        if len(pixel_points) > 1:
            draw.line(pixel_points, fill=255, width=1)
        return np.array(im, dtype=np.float64)

    def calculate_correlation(self, img_a, img_b):
        # Flatten and compute spatial Pearson correlation coefficient
        flat_a = img_a.flatten()
        flat_b = img_b.flatten()
        std_a = np.std(flat_a)
        std_b = np.std(flat_b)
        if std_a < 1e-6 or std_b < 1e-6:
            return 0.0
        cov = np.mean((flat_a - np.mean(flat_a)) * (flat_b - np.mean(flat_b)))
        return cov / (std_a * std_b)

    def verify_frame_mix(self, master_points, pre_master_templates):
        """
        Correlates the master attractor shape against pre-master templates to
        visually identify instrument presence on the substrate.
        """
        master_img = self.rasterize_coordinates(master_points)
        correlations = {}
        
        for name, pts in pre_master_templates.items():
            tpl_img = self.rasterize_coordinates(pts)
            corr = self.calculate_correlation(master_img, tpl_img)
            correlations[name] = max(0.0, corr)
            
        # Normalize correlations to estimate mix ratios
        total = sum(correlations.values())
        if total > 0.0:
            mix_ratios = {name: val / total for name, val in correlations.items()}
        else:
            mix_ratios = {name: 0.0 for name in correlations.keys()}
            
        # Classify dominant instrument
        dominant = max(correlations, key=correlations.get) if total > 0.0 else "SILENT"
        
        return mix_ratios, dominant

def test_optical_transducer():
    print("[OPTICAL-TRANSDUCER] Initializing validation sweep...")
    
    # Generate mock coordinate traces representing different shapes
    theta = np.linspace(0, 2*np.pi, 500)
    
    # Pre-master template shapes
    templates = {
        "sub_growl": [(30 * np.cos(t), 30 * np.sin(t)) for t in theta],
        "lead_synth": [(40 * np.cos(3*t), 40 * np.sin(3*t)) for t in theta],
        "snare_drum": [(25 * np.cos(5*t), 20 * np.sin(2*t)) for t in theta]
    }
    
    transducer = OpticalVerificationTransducer()
    
    # Test case 1: Master is identical to sub_growl
    mix, dom = transducer.verify_frame_mix(templates["sub_growl"], templates)
    print(f"Case 1 (Master is Growl) | Dominant: {dom:<12} | Mix: {dict((k, f'{v:.2%}') for k, v in mix.items())}")
    assert dom == "sub_growl"
    
    # Test case 2: Master is identical to lead_synth
    mix, dom = transducer.verify_frame_mix(templates["lead_synth"], templates)
    print(f"Case 2 (Master is Lead)  | Dominant: {dom:<12} | Mix: {dict((k, f'{v:.2%}') for k, v in mix.items())}")
    assert dom == "lead_synth"
    
    print("[SUCCESS] Optical Verification Transducer tests passed.")

if __name__ == "__main__":
    test_optical_transducer()
