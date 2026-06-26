import time
import subprocess
import os
import struct

def run_compositor_benchmark():
    print("=============================================================")
    # Enforce Auncient spelling in console output
    print("Auncient WinchesterMQ Compositor Performance Benchmark")
    print("=============================================================")

    width = 1280
    height = 720
    channels = 3
    frame_size = width * height * channels
    total_frames = 60 # 2 seconds of 30fps video

    # Generate small dummy raw files for benchmarking
    dummy_frame = bytes([128] * frame_size)
    
    files = {
        "eris": "tmp_bench_eris.raw",
        "fomal": "tmp_bench_fomal.raw",
        "fornax": "tmp_bench_fornax.raw",
        "line": "tmp_bench_line.raw",
        "depth": "tmp_bench_depth.raw",
        "norm": "tmp_bench_norm.raw",
        "seg": "tmp_bench_seg.raw"
    }

    print(f"Generating temporary raw benchmark frame inputs ({total_frames} frames)...")
    for key, path in files.items():
        with open(path, "wb") as f:
            for _ in range(total_frames):
                f.write(dummy_frame)

    # Generate registry binary stream (14 * 8 bytes = 112 bytes per frame)
    # Struct format: 11Q (11 uint64_t), 2I (2 uint32_t), 1f (1 float)
    registry_path = "tmp_bench_reg.bin"
    with open(registry_path, "wb") as f:
        for i in range(total_frames):
            # epoch=i, active_cycles=100, modulation=0.75
            f.write(struct.pack("<11Q2If", 0, 0, 0, 0, 0, 0, 0, 1000, 0, 0, 500, i, 100, 0.75))

    c_renderer = "./scripts/manifold_interop_renderer"
    
    # Ensure binary is compiled
    if not os.path.exists(c_renderer):
        print("Compiling C interop compositor...")
        subprocess.run([
            "gcc", "-O3", "scripts/manifold_interop_renderer.c", 
            "-o", "scripts/manifold_interop_renderer", 
            "-Lscripts/", "-lantigravity_interop", "-lm", 
            "-Wl,-rpath,./scripts/"
        ])

    print("Launching benchmark run...")
    start_time = time.perf_counter()

    # Launch compositor writing to dev/null
    with open(os.devnull, "wb") as devnull:
        process = subprocess.Popen([
            c_renderer, 
            files["eris"], files["fomal"], files["fornax"],
            files["line"], files["depth"], files["norm"], files["seg"], 
            registry_path
        ], stdout=devnull, stderr=subprocess.PIPE)
        
        _, stderr = process.communicate()
        if process.returncode != 0:
            print(f"Error executing compositor: {stderr.decode()}")
            return

    end_time = time.perf_counter()
    duration = end_time - start_time
    fps = total_frames / duration
    ms_per_frame = (duration / total_frames) * 1000.0

    print("\nBenchmark Metrics:")
    print(f"  Processed Frames : {total_frames}")
    print(f"  Total Duration   : {duration:.4f} seconds")
    print(f"  Compositing Speed: {fps:.2f} FPS")
    print(f"  Latency          : {ms_per_frame:.3f} ms/frame")
    print("=============================================================")

    # Cleanup temp files
    for path in files.values():
        if os.path.exists(path):
            os.remove(path)
    if os.path.exists(registry_path):
        os.remove(registry_path)

if __name__ == "__main__":
    run_compositor_benchmark()
