import unittest
import os
import struct
import math
import sys

# Add root directory to path to import compile_dna
sys.path.append(os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))
import compile_dna

class TestDnaParameters(unittest.TestCase):
    
    def test_preset_integrity(self):
        """Verify that both presets contain all necessary genomic parameters."""
        required_keys = [
            "cranial_scale", "body_fatness", "tail_length", 
            "muscle_stiffness", "walk_frequency", "breathing_frequency",
            "biotika_membrane_threshold", "sickness_resistance",
            "base_fur_color", "eye_accent_color"
        ]
        for preset_name, genome in compile_dna.PRESETS.items():
            for key in required_keys:
                self.assertIn(key, genome, f"Preset '{preset_name}' is missing parameter '{key}'")

    def test_parameter_boundary_limits(self):
        """Verify sanity limits for each genome parameter in both presets."""
        for preset_name, genome in compile_dna.PRESETS.items():
            # Cranial scale must be a positive float within anatomical bounds
            self.assertTrue(0.1 <= genome["cranial_scale"] <= 3.0, 
                            f"cranial_scale boundary violated in {preset_name}")
            
            # Body fatness must be a positive float
            self.assertTrue(0.1 <= genome["body_fatness"] <= 3.0, 
                            f"body_fatness boundary violated in {preset_name}")
            
            # Tail length cannot be negative
            self.assertTrue(0.0 <= genome["tail_length"] <= 5.0, 
                            f"tail_length boundary violated in {preset_name}")
            
            # Muscle stiffness acts as a damping decay rate; must be strictly within (0.0, 1.0)
            self.assertTrue(0.0 < genome["muscle_stiffness"] < 1.0, 
                            f"muscle_stiffness damping bounds violated in {preset_name}")
            
            # Walk frequency should be positive and within stable integration bounds
            self.assertTrue(0.05 <= genome["walk_frequency"] <= 2.5, 
                            f"walk_frequency boundary violated in {preset_name}")
            
            # Breathing frequency should be positive
            self.assertTrue(5.0 <= genome["breathing_frequency"] <= 100.0, 
                            f"breathing_frequency boundary violated in {preset_name}")
            
            # Membrane threshold for spiking
            self.assertTrue(0.1 <= genome["biotika_membrane_threshold"] <= 2.0, 
                            f"biotika_membrane_threshold boundary violated in {preset_name}")
            
            # Sickness resistance
            self.assertTrue(0.0 <= genome["sickness_resistance"] <= 1.0, 
                            f"sickness_resistance boundary violated in {preset_name}")
            
            # RGB Colors must be 3-tuples and within [0, 255]
            for color_key in ["base_fur_color", "eye_accent_color"]:
                color = genome[color_key]
                self.assertEqual(len(color), 3, f"{color_key} must have exactly 3 channels in {preset_name}")
                for channel in color:
                    self.assertTrue(0 <= channel <= 255, f"{color_key} channel value {channel} out of bounds in {preset_name}")

    def test_dna_compilation_size(self):
        """Test compile_dna execution and check the generated binary asset structure."""
        # Save active preset to restore later
        original_preset = compile_dna.ACTIVE_PRESET
        
        try:
            for preset_name in ["slugcat", "teddy"]:
                compile_dna.ACTIVE_PRESET = preset_name
                compile_dna.GENOME = compile_dna.PRESETS[preset_name]
                
                # Execute DNA compilation
                compile_dna.compile_dna()
                
                dna_path = "tsfi2-deepseek/assets/atropa.dna"
                self.assertTrue(os.path.exists(dna_path), f"Failed to generate {dna_path} for preset {preset_name}")
                
                # Calculate expected size:
                # 8 bytes header + (10800 frames * 31 bytes/frame) = 334808 bytes
                expected_size = 334808
                actual_size = os.path.getsize(dna_path)
                self.assertEqual(actual_size, expected_size, 
                                 f"DNA binary size mismatch for {preset_name}. Expected {expected_size} bytes, got {actual_size} bytes.")
                
                # Verify magic bytes header
                with open(dna_path, "rb") as f:
                    magic = f.read(4)
                    self.assertEqual(magic, b'TSFI', f"Magic header mismatch for {preset_name}")
                    frames = struct.unpack('=I', f.read(4))[0]
                    self.assertEqual(frames, 10800, f"Frame count mismatch for {preset_name}")
                    
        finally:
            # Restore original active preset
            compile_dna.ACTIVE_PRESET = original_preset
            compile_dna.GENOME = compile_dna.PRESETS[original_preset]
            compile_dna.compile_dna()

if __name__ == "__main__":
    unittest.main()
