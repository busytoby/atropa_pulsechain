import os
import math
import numpy as np

SAMPLE_RATE = 44100
dt = 1.0 / SAMPLE_RATE

def simulate_duffing(delta, beta, alpha, gamma, freq, duration_sec):
    num_samples = int(SAMPLE_RATE * duration_sec)
    y = np.zeros(num_samples)
    y_prev = 0.0
    y_curr = 0.1  # starting displacement
    
    omega = 2.0 * np.pi * freq
    
    for t in range(1, num_samples):
        # Driving force
        force = gamma * math.cos(omega * t * dt)
        
        # Velocity approximation (backward difference)
        v = (y_curr - y_prev) / dt
        
        # Duffing acceleration equation
        acc = -beta * y_curr - alpha * (y_curr ** 3) - delta * v + force
        
        # Verlet integration
        y_next = 2.0 * y_curr - y_prev + acc * (dt ** 2)
        
        y_prev = y_curr
        # Clamp to avoid numerical explosion under extreme chaotic regimes
        y_curr = max(-5.0, min(5.0, y_next))
        
        y[t] = y_curr
        
    return y

def analyze_stability(y):
    # Quantify orbit variation as proxy for chaos (standard deviation of peak amplitude deviations)
    # Find peaks
    peaks = []
    for i in range(1, len(y) - 1):
        if y[i] > y[i-1] and y[i] > y[i+1] and y[i] > 0.05:
            peaks.append(y[i])
            
    if len(peaks) < 5:
        return 0.0 # Not enough oscillations
        
    # Standard deviation of peak values indicates chaotic variation
    # Stable periodic orbits will have near-zero peak deviation
    peak_std = np.std(peaks[-20:])
    return peak_std

def main():
    print("[DSP] Starting programmatically swept Duffing Oscillator Verlet analysis...")
    
    beta = 1.0
    alpha = 1.5
    gamma = 0.35
    freq = 80.0 # Driving frequency at 80Hz
    duration_per_step = 0.5
    
    deltas = np.linspace(0.02, 0.4, 20)
    full_sweep_audio = []
    
    report_lines = []
    report_lines.append("| Delta (Damping) | Peak Deviation (Chaos Metric) | Regime Class |")
    report_lines.append("|-----------------|-------------------------------|--------------|")
    
    for delta in deltas:
        y = simulate_duffing(delta, beta, alpha, gamma, freq, duration_per_step)
        
        # Normalize each block to prevent clipping, but preserve quietness if amplitude is tiny
        max_val = np.max(np.abs(y))
        if max_val > 0.01:
            y_norm = y / max_val
        else:
            y_norm = y
            
        full_sweep_audio.extend(y_norm)
        
        chaos_metric = analyze_stability(y)
        regime = "Chaotic" if chaos_metric > 0.01 else "Stable Periodic"
        report_lines.append(f"| {delta:.4f} | {chaos_metric:.6f} | {regime} |")
        
    # Save Report
    report_content = "\n".join(report_lines)
    print("\n[STABILITY REPORT]\n" + report_content + "\n")
    
    # Save WAV file of the sweep
    audio_arr = np.array(full_sweep_audio)
    audio_arr /= np.max(np.abs(audio_arr))
    
    artifact_dir = "/home/mariarahel/.gemini/antigravity-cli/brain/7445a817-72b7-467a-ae12-acda8b6b2353"
    wav_path = os.path.join(artifact_dir, "verlet_duffing_sweep.wav")
    
    # Save mono 16-bit PCM WAV
    byte_data = bytearray(44 + len(audio_arr) * 2)
    byte_data[0:4] = b"RIFF"
    byte_data[4:8] = int(36 + len(audio_arr) * 2).to_bytes(4, "little")
    byte_data[8:12] = b"WAVE"
    byte_data[12:16] = b"fmt "
    byte_data[16:20] = int(16).to_bytes(4, "little")
    byte_data[20:22] = int(1).to_bytes(2, "little")
    byte_data[22:24] = int(1).to_bytes(2, "little")
    byte_data[24:28] = int(SAMPLE_RATE).to_bytes(4, "little")
    byte_data[28:32] = int(SAMPLE_RATE * 2).to_bytes(4, "little")
    byte_data[32:34] = int(2).to_bytes(2, "little")
    byte_data[34:36] = int(16).to_bytes(2, "little")
    byte_data[36:40] = b"data"
    byte_data[40:44] = int(len(audio_arr) * 2).to_bytes(4, "little")
    
    offset = 44
    for val in audio_arr:
        v = int(val * 32767) if val >= 0 else int(val * 32768)
        byte_data[offset:offset+2] = v.to_bytes(2, "little", signed=True)
        offset += 2
        
    with open(wav_path, "wb") as f:
        f.write(byte_data)
        
    print(f"[SUCCESS] Saved comparative sweep WAV to: {wav_path}")
    
    # Save markdown report to artifact dir
    report_path = os.path.join(artifact_dir, "verlet_duffing_stability_report.md")
    with open(report_path, "w") as f:
        f.write(f"# Verlet Duffing Stability Sweep Report\n\nSweep parameters:\n* alpha = {alpha}\n* beta = {beta}\n* gamma = {gamma}\n* freq = {freq} Hz\n\n" + report_content)
    print(f"[SUCCESS] Saved Markdown report to: {report_path}")

if __name__ == "__main__":
    main()
