import subprocess
import sys

vlms = ["moondream", "qwen2-vl", "llama3.2-vision", "claude", "gemini", "mock"]
generators = ["sd15", "turbo", "dream", "flux", "sdxl"]

print("=== Running Complete Optimizer Combinations Test ===")
failures = 0
tests_run = 0

for vlm in vlms:
    for generator in generators:
        print(f"\n[Test] Target: golden | Critic: {vlm} | Generator: {generator}")
        cmd = [
            "python3", "../scripts/genetic_teddy_optimizer.py",
            "golden",
            "--vlm", vlm,
            "--generator", generator,
            "--max-iterations", "1"
        ]
        res = subprocess.run(cmd, capture_output=True, text=True)
        tests_run += 1
        if res.returncode == 0:
            print(f"  -> SUCCESS")
        else:
            print(f"  -> FAILED (exit code: {res.returncode})")
            print(f"Error output:\n{res.stderr}")
            failures += 1

print("\n=== Test Summary ===")
print(f"Total Tests Run: {tests_run}")
print(f"Successful: {tests_run - failures}")
print(f"Failures: {failures}")

if failures > 0:
    sys.exit(1)
sys.exit(0)
