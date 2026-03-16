import os
import subprocess
import sys

def run_rigid_build(source_file):
    print(f"=== TSFi Rigid Builder: Vetting '{source_file}' ===")
    shadow_dir = "tmp_rigidity_shadow"
    if not os.path.exists(shadow_dir):
        os.makedirs(shadow_dir)
    
    obj_out = os.path.join(shadow_dir, "shadow.o")
    cmd = [
        "gcc", "-Wall", "-Wextra", "-Werror", "-std=c11", 
        "-D_POSIX_C_SOURCE=200809L", "-D_DEFAULT_SOURCE", "-Iinc", "-Iplugins", 
        "-Iplugins/vulkan", "-Iplugins/window_inc", "-Iplugins/window_src",
        "-O3", "-march=native", "-flax-vector-conversions",
        "-c", "-o", obj_out, source_file
    ]
    
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode == 0:
        print("  -> [RIGID] Build is frictionless. 0 Warnings. 0 Errors.")
        return True
    else:
        print(f"  -> [FRACTURE] Friction Units detected!")
        print(result.stderr)
        return False

if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(1)
    run_rigid_build(sys.argv[1])
