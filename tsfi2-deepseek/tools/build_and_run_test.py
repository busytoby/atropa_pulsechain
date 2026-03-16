import glob
import subprocess
import sys

def main():
    objs = glob.glob("obj/*.o")
    # Filter out mains and clashing binaries
    filtered_objs = [o for o in objs if not any(x in o for x in [
        "main.o", "tsfi_mcp_server.o", "tsfi_poll.o", "tsfi_cockpit.o", 
        "tsfi_time_cli.o", "tsfi_font_editor.o", "tsfi_teacher.o", 
        "tsfi_discovery.o", "tsfi_fetch.o", "bench_reflection.o"
    ])]

    cmd = ["gcc", "-Iinc", "-Iplugins", "-Iplugins/vulkan", "-mavx512f", "-mclflushopt", 
           "-o", "bin/test_soft_encode_leak", "tests/test_soft_encode_leak.c"] + filtered_objs + [
           "-lrt", "-lubsan", "-lwayland-client", "-lvulkan"]
           
    print("Compiling test...")
    try:
        subprocess.run(cmd, check=True)
        print("Running test...")
        subprocess.run(["./bin/test_soft_encode_leak"], check=True)
    except subprocess.CalledProcessError as e:
        print(f"Failed: {e}")

if __name__ == "__main__":
    main()
