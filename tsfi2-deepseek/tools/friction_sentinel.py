import os
import subprocess
import sys

def check_syntax(file_path):
    print(f"[SENTINEL] Syntax Audit: {file_path}")
    cmd = [
        "gcc", "-fsyntax-only", "-Wall", "-Wextra", "-Werror", 
        "-std=c11", "-D_POSIX_C_SOURCE=200809L", 
        "-Iinc", "-Iplugins", "-Iplugins/vulkan", 
        "-Iplugins/window_inc", "-Iplugins/window_src", 
        file_path
    ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode == 0:
        print("  -> [RIGID] Syntax is valid.")
        return True
    else:
        print("[FRACTURE] Syntax error detected:")
        print(result.stderr)
        return False

if __name__ == "__main__":
    if len(sys.argv) < 2:
        sys.exit(1)
    check_syntax(sys.argv[1])
