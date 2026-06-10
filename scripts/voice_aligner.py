import sys
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

def hz_to_mel(hz):
    return 2595.0 * np.log10(1.0 + hz / 700.0)

def mel_to_hz(mel):
    return 700.0 * (10.0**(mel / 2595.0) - 1.0)

def get_mel_filterbank(num_filters, fft_size, sample_rate):
    low_mel = hz_to_mel(0)
    high_mel = hz_to_mel(sample_rate / 2.0)
    mel_pts = np.linspace(low_mel, high_mel, num_filters + 2)
    hz_pts = mel_to_hz(mel_pts)
    
    bin_pts = np.floor((fft_size + 1) * hz_pts / sample_rate).astype(int)
    
    filters = np.zeros((num_filters, fft_size // 2 + 1))
    for m in range(1, num_filters + 1):
        f_m_minus = bin_pts[m - 1]
        f_m = bin_pts[m]
        f_m_plus = bin_pts[m + 1]
        
        for k in range(f_m_minus, f_m):
            if f_m - f_m_minus > 0:
                filters[m - 1, k] = (k - f_m_minus) / (f_m - f_m_minus)
        for k in range(f_m, f_m_plus):
            if f_m_plus - f_m > 0:
                filters[m - 1, k] = (f_m_plus - k) / (f_m_plus - f_m)
                
    return filters

def compute_mfcc(samples, sample_rate, frame_size=512, hop_size=256, num_mel=20, num_ceps=12):
    frames = []
    for i in range(0, len(samples) - frame_size, hop_size):
        frame = samples[i:i+frame_size]
        window = np.hanning(frame_size)
        windowed = frame * window
        power = np.abs(np.fft.rfft(windowed)) ** 2
        frames.append(power)
    
    if not frames:
        return np.zeros((0, num_ceps))
        
    specs = np.array(frames)
    fb = get_mel_filterbank(num_mel, frame_size, sample_rate)
    mel_specs = np.dot(specs, fb.T)
    mel_specs = np.log(mel_specs + 1e-8)
    
    # Discrete Cosine Transform (Type-II)
    mfccs = np.zeros((mel_specs.shape[0], num_ceps))
    for n in range(num_ceps):
        basis = np.cos(np.pi * n * (np.arange(num_mel) + 0.5) / num_mel)
        mfccs[:, n] = np.dot(mel_specs, basis)
        
    return mfccs

def main():
    if len(sys.argv) < 3:
        print("Usage: voice_aligner.py <file1.wav> <file2.wav>")
        sys.exit(1)
        
    try:
        s1, r1 = load_wav(sys.argv[1])
        s2, r2 = load_wav(sys.argv[2])
    except Exception as e:
        print(f"Error loading WAV files: {e}")
        sys.exit(1)
        
    mfcc1 = compute_mfcc(s1, r1)
    mfcc2 = compute_mfcc(s2, r2)
    
    min_len = min(len(mfcc1), len(mfcc2))
    max_len = max(len(mfcc1), len(mfcc2))
    
    if min_len == 0:
        print("Error: Empty MFCC frames")
        sys.exit(1)
        
    diff = mfcc1[:min_len] - mfcc2[:min_len]
    # Standard Mel-Cepstral Distortion (MCD) calculation
    mcd = np.mean(np.sqrt(np.sum(diff ** 2, axis=1))) * (10.0 / np.log(10.0)) * 0.1
    
    # Smooth gradient: maps MCD = 0.0 -> 100%, MCD = 3.0 -> 25%, MCD = 99.0 -> 1%
    spectral_similarity = 100.0 / (1.0 + mcd)
    duration_ratio = min_len / max_len
    similarity = spectral_similarity * duration_ratio
    
    print(f"File 1 frames: {len(mfcc1)}, File 2 frames: {len(mfcc2)} (Ratio: {duration_ratio:.5f})")
    print(f"MCD_DISTANCE:{mcd:.4f} dB")
    print(f"SPECTRAL_SIMILARITY:{spectral_similarity:.4f}")
    print(f"SIMILARITY_SCORE:{similarity:.4f}")

if __name__ == '__main__':
    main()
