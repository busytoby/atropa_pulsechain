import unittest
import time
import os
import sys
import subprocess

# Ensure we can import from scripts directory
sys.path.append(os.path.join(os.path.dirname(__file__), ".."))

class TestDefaultRenders(unittest.TestCase):
    def setUp(self):
        # Clean up any previously generated assets to ensure fresh rendering
        self.addresses = [
            "0x008fc4bbb1998bfee060f780be7688f0cec66bff", # BZO
            "0x01421a507c4d4685f53790d8855d594cde096183", # PO0P
            "0x041a80b38d3a5b4dbb30e56440ca8f0c8dfa6412"  # SAV
        ]
        for addr in self.addresses:
            path = f"assets/{addr.lower()}.png"
            if os.path.exists(path):
                os.remove(path)

    def test_default_renders_time_and_success(self):
        from scripts.render_vlm_synthesizer import render_vlm_synthesized_frame
        
        for addr in self.addresses:
            print(f"\n--- Testing Render for Address: {addr} ---")
            start_time = time.time()
            
            # Run the render function with 2 steps to ensure it completes very quickly (under 5 seconds)
            # using same default parameters as the frontend / test loops
            render_vlm_synthesized_frame(
                frame_idx=777,
                steps=2,
                cfg=7.5,
                prompt_override=None,
                address=addr,
                hypobar=0,
                epibar=0,
                is_deep_render=True
            )
            
            elapsed = time.time() - start_time
            print(f"Render completed in {elapsed:.3f} seconds.")
            
            # Assert execution time is less than 5 seconds
            self.assertLess(elapsed, 5.0, f"Render for {addr} took too long: {elapsed:.3f}s")
            
            # Assert output file exists and has content
            out_file = f"assets/{addr.lower()}.png"
            self.assertTrue(os.path.exists(out_file), f"Output image was not generated: {out_file}")
            self.assertGreater(os.path.getsize(out_file), 0, f"Generated image file is empty: {out_file}")
            
            # Verify the log outputs by running the CLI tool to double check no fallback errors occurred
            cmd = [
                sys.executable,
                "scripts/render_vlm_synthesizer.py",
                "777",
                "--steps", "2",
                "--cfg", "7.5",
                "--address", addr,
                "--is-deep-render"
            ]
            
            result = subprocess.run(cmd, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, f"CLI render execution failed: {result.stderr}")
            self.assertNotIn("Broken pipe", result.stdout, "Broken pipe detected in render stdout")
            self.assertNotIn("Broken pipe", result.stderr, "Broken pipe detected in render stderr")
            self.assertNotIn("SD worker failed", result.stdout, "Stable Diffusion worker failed fallback check")

if __name__ == "__main__":
    unittest.main()
