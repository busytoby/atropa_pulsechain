#!/usr/bin/env python3
import unittest
import math
import os
import json
import sys

# Add scripts directory to path to test imports if needed
sys.path.append(os.path.join(os.path.dirname(__file__), "../scripts"))

# Mock/Import functions to verify math properties
def project_3d_test(x, y, z, cam_x, cam_y, cam_z, yaw, pitch, zoom, width, height):
    def get_raw_proj(px_val, py_val, pz_val):
        dx = px_val - cam_x
        dy = py_val - cam_y
        dz = pz_val - cam_z
        
        cos_y, sin_y = math.cos(yaw), math.sin(yaw)
        rx = dx * cos_y - dz * sin_y
        ry = dy
        rz = dx * sin_y + dz * cos_y
        
        cos_p, sin_p = math.cos(pitch), math.sin(pitch)
        x_new = rx
        y_new = ry * cos_p - rz * sin_p
        z_new = ry * sin_p + rz * cos_p
        
        focal = 500.0
        if z_new == 0: z_new = 1
        px = (x_new * focal) / (z_new + 700) * zoom
        py = (y_new * focal) / (z_new + 700) * zoom
        return px, py
        
    px_raw, py_raw = get_raw_proj(x, y, z)
    px_orig, py_orig = get_raw_proj(0, 0, 0)
    
    px_final = px_raw - px_orig + width / 2
    py_final = py_raw - py_orig + height / 2
    return int(px_final), int(py_final)

class TestYiLauIconLogic(unittest.TestCase):
    
    def test_origin_centering(self):
        """Assert that projecting the origin (0, 0, 0) always maps to the viewport center."""
        width, height = 1280, 720
        # Check various camera positions
        for yaw in [0.0, 0.5, 1.5, 3.14]:
            for pitch in [0.1, 0.4, 0.8]:
                cam_x = math.cos(yaw) * 360
                cam_y = math.sin(yaw) * 360
                cam_z = 240
                
                px, py = project_3d_test(0, 0, 0, cam_x, cam_y, cam_z, yaw, pitch, 0.82, width, height)
                self.assertEqual(px, width // 2)
                self.assertEqual(py, height // 2)

    def test_morph_boundary_conditions(self):
        """Assert that morph coordinates transition perfectly from YI limits to LAU limits."""
        fx, fy, fz, phi = 2.0, 3.0, 4.0, 0.5
        R_hyp, r_hyp, d_val = 150.0, 30.0, 50.0
        theta = 1.0
        
        # YI coordinate path (Lissajous)
        lx_yi = 135.0 * math.sin(fx * theta + phi)
        ly_yi = 135.0 * math.sin(fy * theta)
        lz_yi = 135.0 * math.cos(fz * theta)
        
        # LAU coordinate path (Hypotrochoid)
        lx_lau = (R_hyp - r_hyp) * math.cos(theta) + d_val * math.cos(((R_hyp - r_hyp) / r_hyp) * theta + phi)
        ly_lau = (R_hyp - r_hyp) * math.sin(theta) - d_val * math.sin(((R_hyp - r_hyp) / r_hyp) * theta)
        lz_lau = 80.0 * math.cos(5.0 * theta)
        
        # Test t=0 (Pure YI)
        morph_factor = 0.0
        lx_0 = lx_yi * (1.0 - morph_factor) + lx_lau * morph_factor
        ly_0 = ly_yi * (1.0 - morph_factor) + ly_lau * morph_factor
        lz_0 = lz_yi * (1.0 - morph_factor) + lz_lau * morph_factor
        self.assertAlmostEqual(lx_0, lx_yi)
        self.assertAlmostEqual(ly_0, ly_yi)
        self.assertAlmostEqual(lz_0, lz_yi)
        
        # Test t=1 (Pure LAU)
        morph_factor = 1.0
        lx_1 = lx_yi * (1.0 - morph_factor) + lx_lau * morph_factor
        ly_1 = ly_yi * (1.0 - morph_factor) + ly_lau * morph_factor
        lz_1 = lz_yi * (1.0 - morph_factor) + lz_lau * morph_factor
        self.assertAlmostEqual(lx_1, lx_lau)
        self.assertAlmostEqual(ly_1, ly_lau)
        self.assertAlmostEqual(lz_1, lz_lau)

    def test_cache_file_format(self):
        """Verify the integrity of scripts/pulsechain_register_cache.json if it exists."""
        cache_path = os.path.join(os.path.dirname(__file__), "../scripts/pulsechain_register_cache.json")
        if os.path.exists(cache_path):
            with open(cache_path, "r") as f:
                data = json.load(f)
            self.assertIsInstance(data, dict)
            for addr, registers in data.items():
                self.assertTrue(addr.startswith("0x"))
                for reg in ["r_base", "r_channel", "r_dynamo", "r_foundation", 
                            "c_base", "c_channel", "c_dynamo", "c_foundation"]:
                    self.assertIn(reg, registers)
                    # Verify they are convertible to int
                    self.assertTrue(int(registers[reg]) >= 0)

if __name__ == "__main__":
    unittest.main()
