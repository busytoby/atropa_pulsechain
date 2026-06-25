import os
import math
import wave
import struct
import numpy as np

SAMPLE_RATE = 44100
dt = 1.0 / SAMPLE_RATE

# Pre-calculated Sine Lookup Table for fast DSP oscillation
SINE_LUT_SIZE = 8192
SINE_LUT = np.sin(np.linspace(0, 2.0 * math.pi, SINE_LUT_SIZE, endpoint=False))

def fast_sin(x):
    # Lookup index wrapping matching period
    idx = int((x % (2.0 * math.pi)) * (SINE_LUT_SIZE / (2.0 * math.pi)))
    return SINE_LUT[idx % SINE_LUT_SIZE]

# Phoneme formant mapping for speech synthesis
PHONEME_TABLE = {
    # Vowels (F1, F2, F3, is_voiced)
    "a": (730.0, 1090.0, 2440.0, True),
    "e": (270.0, 2290.0, 3010.0, True),
    "i": (300.0, 2200.0, 3000.0, True),
    "o": (570.0, 840.0, 2400.0, True),
    "u": (300.0, 870.0, 2240.0, True),
    # Consonants (noise or transient frequencies)
    "s": (6000.0, 8000.0, 10000.0, False),
    "f": (2200.0, 4000.0, 6000.0, False),
    "t": (1500.0, 3000.0, 5000.0, False),  # transient burst
    "n": (300.0, 1400.0, 2700.0, True),
    "m": (300.0, 1100.0, 2500.0, True),
    "g": (200.0, 1600.0, 2400.0, True),
    "r": (310.0, 1060.0, 1670.0, True),
    "v": (250.0, 1500.0, 2500.0, True),
    "y": (300.0, 2200.0, 3000.0, True),
    "ng": (300.0, 1800.0, 2700.0, True),  # nasal ng
    "un": (400.0, 1500.0, 2500.0, True),  # nasal un
    " ": (0.0, 0.0, 0.0, False),  # silence
}

def text_to_phonemes(text):
    """
    Convert raw text to phonetic symbols defined in PHONEME_TABLE.
    Supports basic G2P rules for English/latin spelling representation.
    """
    text = text.lower()
    phonemes = []
    i = 0
    while i < len(text):
        char = text[i]
        # Match multi-character rules
        if i + 1 < len(text):
            sh = text[i:i+2]
            if sh in ["sh", "ch"]:
                phonemes.append("s")
                i += 2
                continue
            if sh == "ee":
                phonemes.append("e")
                i += 2
                continue
            if sh == "oo":
                phonemes.append("u")
                i += 2
                continue
            if sh == "th":
                phonemes.append("f")
                i += 2
                continue
            if sh == "ng":
                phonemes.append("ng")
                i += 2
                continue
            if sh == "un":
                phonemes.append("un")
                i += 2
                continue
            if sh == "an":
                phonemes.append("un")
                i += 2
                continue

        if char in PHONEME_TABLE:
            phonemes.append(char)
        elif char.isspace():
            phonemes.append(" ")
        i += 1
    return phonemes

def generate_syrinx_speech(text, output_wav="bionika_syrinx_speech.wav"):
    phonemes = text_to_phonemes(text)
    if not phonemes:
        phonemes = [" "]
        
    # Assign durations to each phoneme
    phoneme_duration = 0.20  # seconds per phoneme
    total_duration = len(phonemes) * phoneme_duration
    num_samples = int(SAMPLE_RATE * total_duration)
    
    out_signal = np.zeros(num_samples)
    
    # 2-Mass coupled biological membrane Verlet state (initialized to zero)
    x1, x1_prev = 0.0, 0.0
    x2, x2_prev = 0.0, 0.0
    
    # Syrinx mass parameters (whistle/growl hybrid for vocalization)
    m1 = 0.07
    m2 = 0.05
    c1 = 0.8
    c2 = c1 * 0.8
    Kc = 8.0
    Ps = 0.35
    
    # Bionika Spiking Neuron states
    bionika_vm = 0.0
    bionika_leak = 0.95
    bionika_threshold = 1.0
    
    # Acoustic tract load
    r_tract = 0.95
    y_tract, y_tract_prev = 0.0, 0.0
    
    # Random source for unvoiced consonants
    noise = np.random.uniform(-1.0, 1.0, num_samples)
    
    print(f"[DSP] Generating syrinx speech for: '{text}'")
    print(f"[DSP] Resolved Phonemes: {phonemes}")
    
    for s in range(num_samples):
        t_curr = s * dt
        
        # Determine active phoneme index
        ph_idx = min(len(phonemes) - 1, int(t_curr / phoneme_duration))
        curr_ph = phonemes[ph_idx]
        
        # Smooth interpolation to next phoneme target to prevent click transients
        next_ph_idx = min(len(phonemes) - 1, ph_idx + 1)
        next_ph = phonemes[next_ph_idx]
        ph_progress = (t_curr / phoneme_duration) - ph_idx
        
        f1_curr, f2_curr, f3_curr, is_voiced_curr = PHONEME_TABLE[curr_ph]
        f1_next, f2_next, f3_next, is_voiced_next = PHONEME_TABLE[next_ph]
        
        # Interpolated formants
        f1 = f1_curr * (1.0 - ph_progress) + f1_next * ph_progress
        f2 = f2_curr * (1.0 - ph_progress) + f2_next * ph_progress
        
        is_voiced = is_voiced_curr
        
        # Bionika Spiking Neuron: integrates energy from the output sound flow
        bionika_vm = bionika_vm * bionika_leak + abs(y_tract) * 0.08
        fired = 0
        if bionika_vm >= bionika_threshold:
            bionika_vm = 0.0
            fired = 1
            
        # Neural modulation: Spikes shift base pitch higher, V_m increases tract damping
        pitch_mod = 1.0 + (bionika_vm * 0.15) + (fired * 0.25)
        base_melody_freq = 180.0 * pitch_mod  # base frequency
        
        # Map frequency to stiffness scaling
        stiff_scale = (base_melody_freq / 220.0) ** 2
        epibar = 1200.0 * stiff_scale
        hypobar = 1000.0 * stiff_scale
        
        stiffness1 = epibar if x1 > 0.0 else hypobar
        stiffness2 = epibar * 0.9 if x2 > 0.0 else hypobar * 0.9
        
        # Calculate force
        f_p1 = Ps * 0.2 if x1 > 0.0 else 0.0
        f_p2 = Ps * 0.15 if x2 > 0.0 else 0.0
        
        v1 = (x1 - x1_prev) / dt
        v2 = (x2 - x2_prev) / dt
        
        acc1 = (f_p1 - stiffness1 * x1 - c1 * v1 + Kc * (x2 - x1)) / m1
        acc2 = (f_p2 - stiffness2 * x2 - c2 * v2 + Kc * (x1 - x2)) / m2
        
        # Verlet Integration step
        x1_next = 2.0 * x1 - x1_prev + acc1 * (dt ** 2)
        x2_next = 2.0 * x2 - x2_prev + acc2 * (dt ** 2)
        
        x1_prev, x1 = x1, max(-0.4, min(0.8, x1_next))
        x2_prev, x2 = x2, max(-0.4, min(0.8, x2_next))
        
        flow = (max(x1, 0.0) ** 2)
        
        if is_voiced:
            excitation = flow
        else:
            excitation = noise[s] * 0.35
            
        if f1 > 0:
            # Dual-formant resonance model excited by syrinx membrane flow or friction noise
            formant_excitation = excitation * (
                0.60 * fast_sin(2.0 * math.pi * f1 * t_curr) +
                0.40 * fast_sin(2.0 * math.pi * f2 * t_curr)
            )
        else:
            formant_excitation = 0.0
            
        # Waveguide acoustic resonance filter
        y_next = formant_excitation + r_tract * y_tract - 0.9 * y_tract_prev
        y_tract_prev, y_tract = y_tract, y_next
        
        out_signal[s] = y_next
        
    # High-pass filter to clean up low-frequency rumble
    for s in range(num_samples - 1, 0, -1):
        out_signal[s] = out_signal[s] - 0.95 * out_signal[s-1]
        
    # Normalize signal
    out_signal -= np.mean(out_signal)
    if np.max(np.abs(out_signal)) > 0:
        out_signal /= np.max(np.abs(out_signal))
        
    # Write WAV file
    byte_data = bytearray(44 + len(out_signal) * 2)
    byte_data[0:4] = b"RIFF"
    byte_data[4:8] = int(36 + len(out_signal) * 2).to_bytes(4, "little")
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
    byte_data[40:44] = int(len(out_signal) * 2).to_bytes(4, "little")
    
    offset = 44
    for val in out_signal:
        v = int(val * 32767) if val >= 0 else int(val * 32768)
        byte_data[offset:offset+2] = v.to_bytes(2, "little", signed=True)
        offset += 2
        
    with open(output_wav, "wb") as f:
        f.write(byte_data)
        
    print(f"[SUCCESS] Bionika syrinx speech saved to {output_wav}")

if __name__ == "__main__":
    import sys
    phrase = "auncient bionika speaks with the syrinx"
    if len(sys.argv) > 1:
        phrase = " ".join(sys.argv[1:])
    generate_syrinx_speech(phrase)
