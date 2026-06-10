import subprocess
import random
import sys
import re
import os
import numpy as np

def load_wav(filename):
    with open(filename, 'rb') as f:
        header = f.read(44)
        if len(header) < 44:
            raise ValueError("Invalid WAV file")
        # Extract metadata
        channels = int.from_bytes(header[22:24], 'little')
        sample_rate = int.from_bytes(header[24:28], 'little')
        bits_per_sample = int.from_bytes(header[34:36], 'little')
        data = f.read()
        
        if bits_per_sample == 16:
            samples = np.frombuffer(data, dtype=np.int16).astype(np.float32)
        elif bits_per_sample == 8:
            samples = (np.frombuffer(data, dtype=np.uint8).astype(np.float32) - 128.0) * 256.0
        else:
            raise ValueError(f"Unsupported bits: {bits_per_sample}")
            
        return samples, sample_rate

def extract_pitch_contour(filename, num_frames=24, frame_size=1024):
    try:
        samples, sample_rate = load_wav(filename)
    except Exception as e:
        print(f"Error loading {filename}: {e}")
        return [220] * num_frames

    min_freq = 80
    max_freq = 350
    min_period = int(sample_rate / max_freq)
    max_period = int(sample_rate / min_freq)

    pitches = []
    total_len = len(samples)
    
    for i in range(num_frames):
        start = int(i * (total_len - frame_size) / (num_frames - 1)) if num_frames > 1 else 0
        frame = samples[start : start + frame_size]
        if len(frame) < frame_size:
            frame = np.pad(frame, (0, frame_size - len(frame)))
            
        frame = frame - np.mean(frame)
        
        # Check energy level to ignore silence/noise floor
        if np.max(np.abs(frame)) < 100.0:
            pitches.append(220)  # default pitch for silent frames
            continue
            
        corr = np.correlate(frame, frame, mode='full')
        corr = corr[len(corr)//2 :]
        
        if len(corr) > max_period:
            search_area = corr[min_period:max_period]
            if len(search_area) > 0:
                peak = np.argmax(search_area) + min_period
                pitch = sample_rate / peak
                pitch = max(min_freq, min(max_freq, pitch))
                pitches.append(round(pitch))
            else:
                pitches.append(220)
        else:
            pitches.append(220)
            
    return pitches

def run_synthesis_and_eval(embedding, phonemes, pitch_contour):
    # 1. Run C speech controller with coefficients and pitch contour env var
    cmd = [
        "./tests/test_speech_c_controller",
        "ana",
    ] + [str(c) for c in embedding] + phonemes
    
    env = os.environ.copy()
    env["PITCH_CONTOUR"] = ",".join(map(str, pitch_contour))
    env["SILENT"] = "1"
    
    try:
        # Run C controller headless
        res = subprocess.run(cmd, env=env, cwd="tsfi2-deepseek", capture_output=True, text=True)
        if res.returncode != 0:
            print(f"C synthesis failed: {res.stderr}")
            return 0.0
    except Exception as e:
        print(f"Failed to launch synthesis: {e}")
        return 0.0

    # 2. Run voice_aligner.py to get similarity score
    eval_cmd = [
        "python3",
        "scripts/voice_aligner.py",
        "tsfi2-deepseek/karateka_c_synthesis.wav",
        "hello_synthesis.wav"
    ]
    
    try:
        res = subprocess.run(eval_cmd, capture_output=True, text=True)
        if res.returncode != 0:
            return 0.0
        # Parse score
        match = re.search(r"SPECTRAL_SIMILARITY:([\d\.]+)", res.stdout)
        if match:
            return float(match.group(1))
    except Exception as e:
        print(f"Failed to run alignment evaluator: {e}")
    return 0.0

def main():
    print("=== TSFi Autonomous Speech Training & Optimization Loop (Short Vowel) ===")
    
    ref_path = "tsfi2-deepseek/test_ana.wav"
    print(f"Extracting pitch contour from reference voice: '{ref_path}'...")
    pitch_contour = extract_pitch_contour(ref_path, num_frames=24)
    print(f"Extracted Pitch Contour (24 frames): {pitch_contour}")
    
    yul_calldata = []
    for p in pitch_contour:
        yul_calldata.append(f"0x{p:064x}")
    print("\n--- Yul registerSpeaker Pitch Contour Values (hex words) ---")
    for val in yul_calldata:
        print(val)
    print("------------------------------------------------------------\n")

    # Repeat "h, ee, l, oo" 20 times to get exactly 80 phonemes, matching the 6400 samples of hello_synthesis.wav
    phonemes = ["h", "ee", "l", "oo"] * 20
    
    # Initial 'ana' coefficients
    best_embedding = [15, -25, 40, -10, 5, -8, 12, -5]
    best_score = run_synthesis_and_eval(best_embedding, phonemes, pitch_contour)
    print(f"Initial Baseline Score: {best_score:.4f}% with coefficients {best_embedding}")

    iterations = 50
    step_size = 5
    
    for i in range(iterations):
        candidate = [
            max(-99, min(99, best_embedding[j] + random.choice([-step_size, 0, step_size])))
            for j in range(8)
        ]
        
        score = run_synthesis_and_eval(candidate, phonemes, pitch_contour)
        
        if score > best_score:
            best_score = score
            best_embedding = candidate
            print(f"[Iter {i+1}/{iterations}] NEW BEST! Similarity: {best_score:.4f}% | Embedding: {best_embedding}")
        else:
            if (i + 1) % 10 == 0:
                print(f"[Iter {i+1}/{iterations}] Active training... Best Similarity: {best_score:.4f}%")
                
    print("\n=== Optimization Complete ===")
    print(f"Final Optimized Coefficients: {best_embedding}")
    print(f"Final Best Similarity Score: {best_score:.4f}%")

if __name__ == '__main__':
    main()

