import os
import math
import subprocess
import numpy as np
from PIL import Image, ImageDraw

SAMPLE_RATE = 44100
FPS = 30
BPM = 174
BEAT_DUR = 60.0 / BPM  # ~0.3448 seconds per beat
DURATION = 720.0       # 12 minutes progressive loop
BEAT_LOOP_COUNT = int(DURATION / BEAT_DUR) # 2088 beats

BLOCK_FONT = {
    'T': [
        [1, 1, 1, 1, 1],
        [0, 0, 1, 0, 0],
        [0, 0, 1, 0, 0],
        [0, 0, 1, 0, 0],
        [0, 0, 1, 0, 0]
    ],
    'S': [
        [0, 1, 1, 1, 1],
        [1, 0, 0, 0, 0],
        [0, 1, 1, 1, 0],
        [0, 0, 0, 0, 1],
        [1, 1, 1, 1, 0]
    ],
    'F': [
        [1, 1, 1, 1, 1],
        [1, 0, 0, 0, 0],
        [1, 1, 1, 1, 0],
        [1, 0, 0, 0, 0],
        [1, 0, 0, 0, 0]
    ],
    'i': [
        [0, 0, 1, 0, 0],
        [0, 0, 0, 0, 0],
        [0, 0, 1, 0, 0],
        [0, 0, 1, 0, 0],
        [0, 0, 1, 0, 0]
    ],
    '/': [
        [0, 0, 0, 0, 1],
        [0, 0, 0, 1, 0],
        [0, 0, 1, 0, 0],
        [0, 1, 0, 0, 0],
        [1, 0, 0, 0, 0]
    ],
    '2': [
        [0, 1, 1, 1, 0],
        [0, 0, 0, 0, 1],
        [0, 1, 1, 1, 0],
        [1, 0, 0, 0, 0],
        [1, 1, 1, 1, 1]
    ]
}

def generate_l_system_notes(seed="A", depth=6):
    # production rules for generating self-similar fractal patterns
    rules = {"A": "AB", "B": "CA", "C": "BC"}
    current = seed
    for _ in range(depth):
        next_str = []
        for char in current:
            next_str.append(rules.get(char, char))
        current = "".join(next_str)
    
    # Map A minor harmonic scale steps for Phrygian sound
    note_map = {
        'A': 130.81, # C3
        'B': 146.83, # D3
        'C': 155.56, # Eb3
    }
    return [note_map[char] for char in current[:512]]

def generate_chaos_audio_extended():
    print(f"[DSP] Synthesizing progressive 12-minute loop ({DURATION:.1f}s, {BEAT_LOOP_COUNT} beats)...")
    
    # Synthesize with 2.0s extra trailing headroom for wrapping decay tails
    duration_ext = DURATION + 2.0
    num_samples = int(SAMPLE_RATE * duration_ext)
    t = np.linspace(0, duration_ext, num_samples, endpoint=False)
    
    # 1. Channel 3: Drum & Bass Beat
    drums = np.zeros_like(t)
    beat_samples = int(SAMPLE_RATE * BEAT_DUR)
    num_beats_ext = int(duration_ext / BEAT_DUR)
    

            
    def jfet_saturate(x, boost_db=4.0):
        gain = 10.0 ** (boost_db / 20.0)
        v_in = x * gain
        # Asymmetric JFET quadratic soft-clipping with warmth
        v_out = (v_in - 0.18 * (v_in * v_in)) / (1.0 + 0.65 * np.abs(v_in))
        return np.clip(v_out, -1.0, 1.0)

    for i in range(num_beats_ext + 2):
        onset_sample = int(i * beat_samples)
        time_sec = onset_sample / SAMPLE_RATE
        
        kick_trigger = False
        snare_trigger = False
        hat_trigger = True
        
        # Timeline phases
        if time_sec < 90.0:
            # Complex, syncopated intro beat
            kick_trigger = (i % 8 == 0) or (i % 8 == 6)
            snare_trigger = (i % 8 == 4)
            hat_trigger = (i % 2 == 0) or (i % 8 == 5)
        elif time_sec < 180.0:
            kick_trigger = (i % 4 == 0) or (i % 8 == 2) or (i % 16 == 11)
            snare_trigger = (i % 4 == 2) or (i % 16 == 15)
            hat_trigger = True
        elif time_sec < 270.0:
            kick_trigger = (i % 8 == 0) or (i % 8 == 3) or (i % 8 == 6)
            snare_trigger = (i % 8 == 4)
        elif time_sec < 360.0:
            kick_trigger = (i % 4 == 0) or (i % 8 == 3) or (i % 8 == 5)
            snare_trigger = (i % 4 == 2) or (i % 16 == 15)
        elif time_sec < 450.0:
            # Ambient IDM glitch beat
            kick_trigger = (i % 16 == 0) or (i % 32 == 20)
            snare_trigger = (i % 16 == 12)
            hat_trigger = (i % 4 == 2)
        elif time_sec < 540.0:
            if time_sec < 500.0:
                kick_trigger = (i % 2 == 0)
            else:
                kick_trigger = True
            snare_trigger = False
        elif time_sec < 630.0:
            kick_trigger = (i % 4 == 0) or (i % 8 == 3) or (i % 8 == 5) or (i % 8 == 7)
            snare_trigger = (i % 4 == 2) or (i % 16 == 15) or (i % 32 == 31)
        else:
            hat_trigger = (i % 2 == 0)
            
        if kick_trigger:
            kick_len = int(SAMPLE_RATE * 0.25)
            end_sample = min(num_samples, onset_sample + kick_len)
            k_len = end_sample - onset_sample
            if k_len > 0:
                k_age = np.arange(k_len) / SAMPLE_RATE
                freq = 45.0 + 130.0 * np.exp(-k_age / 0.03)
                kick_val = np.sin(2.0 * np.pi * freq * k_age) * np.exp(-k_age / 0.18)
                drums[onset_sample:end_sample] += jfet_saturate(kick_val * 1.8, boost_db=6.0)
                
        if snare_trigger:
            snare_len = int(SAMPLE_RATE * 0.3)
            end_sample = min(num_samples, onset_sample + snare_len)
            s_len = end_sample - onset_sample
            if s_len > 0:
                s_age = np.arange(s_len) / SAMPLE_RATE
                noise = (np.random.rand(s_len) - 0.5) * 2.0
                
                # Dynamic sweeping resonant lowpass SVF filter to make noise liquid
                f_start = 4500.0
                f_end = 250.0
                fc = f_start * np.exp(-s_age / 0.05) + f_end
                q = 0.35  # Resonant
                low = 0.0
                band = 0.0
                filtered_noise = np.zeros(s_len)
                for n in range(s_len):
                    f_coeff = 2.0 * math.sin(math.pi * fc[n] / SAMPLE_RATE)
                    high = noise[n] - low - q * band
                    band = f_coeff * high + band
                    low = f_coeff * band + low
                    filtered_noise[n] = low
                
                snare_val = filtered_noise * np.exp(-s_age / 0.09)
                snare_val += np.sin(2.0 * np.pi * 180.0 * s_age) * np.exp(-s_age / 0.05)
                drums[onset_sample:end_sample] += jfet_saturate(snare_val * 1.5, boost_db=5.0)
                
        if hat_trigger:
            for half in [0, 0.5]:
                onset_hat = int((i + half) * beat_samples)
                hat_len = int(SAMPLE_RATE * 0.05)
                end_sample = min(num_samples, onset_hat + hat_len)
                h_len = end_sample - onset_hat
                if h_len > 0:
                    h_age = np.arange(h_len) / SAMPLE_RATE
                    noise = (np.random.rand(h_len) - 0.5) * 2.0
                    drums[onset_hat:end_sample] += noise * np.exp(-h_age / 0.015) * 0.08

    # Apply feedback delay to the overall drum mix to add wet decay space
    delay_samples = int(beat_samples * 0.75)
    feedback = 0.38
    drum_delay = np.zeros_like(drums)
    for b in range(1, int(np.ceil(len(drums) / delay_samples))):
        start = b * delay_samples
        end = min(len(drums), (b + 1) * delay_samples)
        drum_delay[start:end] = drums[start-delay_samples:end-delay_samples] + drum_delay[start-delay_samples:end-delay_samples] * feedback
    drums += drum_delay * 0.32

    # -------------------------------------------------------------
    # 2. Technology 2: Cumulative Phase Vectorization (Ch 1 Bassline)
    # -------------------------------------------------------------
    # Instead of step looping, we build a global frequency vector and integrate it
    bass_notes = [65.41, 65.41, 77.78, 58.27, 87.31, 87.31, 73.42, 48.99]
    step_samples = int(beat_samples * 2)
    num_bass_steps = int(num_samples / step_samples)
    
    bass_freq = np.zeros_like(t)
    bass_level_vec = np.zeros_like(t)
    wobble_freq = np.zeros_like(t)
    
    for i in range(num_bass_steps):
        onset = i * step_samples
        time_sec = onset / SAMPLE_RATE
        end = min(num_samples, onset + step_samples)
        b_len = end - onset
        
        note_freq = bass_notes[i % len(bass_notes)]
        bass_level = 0.0
        w_freq = 0.0
        
        if time_sec < 90.0:
            bass_level = 0.35
            w_freq = 0.0
        elif time_sec < 180.0:
            bass_level = 0.45
            w_freq = 5.8
        elif time_sec < 270.0:
            note_freq = note_freq * 0.5
            bass_level = 0.55
            # Swept wobble frequency LFO
            w_freq = 6.0 + 6.0 * np.sin(2.0 * np.pi * 0.25 * (np.arange(b_len) / SAMPLE_RATE))
        elif time_sec < 360.0:
            note_freq = note_freq * 0.5
            bass_level = 0.60
            w_freq = 12.0
        elif time_sec < 450.0:
            note_freq = 32.7
            bass_level = 0.35
            w_freq = 4.0
        elif time_sec < 540.0:
            note_freq = 32.7
            bass_level = 0.40
            w_freq = 3.0 + 5.0 * ((time_sec - 450.0) / 90.0)
        elif time_sec < 630.0:
            note_freq = note_freq * 0.5
            bass_level = 0.65
            w_freq = 14.0
        else:
            note_freq = 32.7
            bass_level = 0.30 * (1.0 - (time_sec - 630.0) / 90.0)
            w_freq = 0.0
            
        if b_len > 0:
            bass_freq[onset:end] = note_freq
            bass_level_vec[onset:end] = bass_level
            if isinstance(w_freq, np.ndarray):
                wobble_freq[onset:end] = w_freq
            else:
                wobble_freq[onset:end] = float(w_freq)
                
    # Integrate frequency to get continuous phase accumulator
    dt_sample = 1.0 / SAMPLE_RATE
    phase_acc = np.cumsum(bass_freq) * dt_sample * 2.0 * np.pi
    
    # Generate oscillators via cumulative phase
    saw = 2.0 * ((phase_acc / (2.0 * np.pi)) % 1.0) - 1.0
    tri = 1.0 - 4.0 * np.abs(((phase_acc / (2.0 * np.pi)) % 1.0) - 0.5)
    
    # Resonant filter sweep simulation
    b_age_global = (np.arange(num_samples) % step_samples) / SAMPLE_RATE
    filter_env = np.exp(-b_age_global / 0.15)
    mix_voice = (saw * 0.3 * filter_env) + (tri * 0.7 * (1.0 - filter_env * 0.5))
    
    # LFO Wobble envelope
    wobble_lfo = 0.5 + 0.5 * np.sin(phase_acc * (wobble_freq / np.maximum(bass_freq, 1.0)))
    wobble_lfo[bass_freq == 0.0] = 1.0
    env = np.exp(-b_age_global / 0.5) * wobble_lfo
    
    bass_raw = np.tanh(mix_voice * (1.5 + env * 4.0)) * bass_level_vec
    
    # Germanium Fuzz Modeling: apply to Phase 4 (270s - 360s) and Phase 7 (540s - 630s)
    fuzz_gain = 3.5
    bias = 0.08
    bass_fuzzed = np.where(bass_raw + bias > 0, 
                           1.0 - np.exp(-(bass_raw + bias) * fuzz_gain), 
                           -1.0 + np.exp((bass_raw + bias) * fuzz_gain * 0.8)) - bias
    
    # Selectively blend the fuzz model over the timeline
    fuzz_envelope = np.zeros_like(t)
    fuzz_envelope[(t >= 270.0) & (t < 360.0)] = 0.85
    fuzz_envelope[(t >= 540.0) & (t < 630.0)] = 0.90
    
    bass = bass_raw * (1.0 - fuzz_envelope) + bass_fuzzed * fuzz_envelope

    # -------------------------------------------------------------
    # 3. Technology 1: Generative L-System Arpeggiator (Ch 2 Lead)
    # -------------------------------------------------------------
    # Generate notes dynamically using production grammars
    lead = np.zeros_like(t)
    arp_notes_l_sys = generate_l_system_notes()
    arp_samples = int(beat_samples / 4)
    num_arp_steps = int(num_samples / arp_samples)
    
    for i in range(num_arp_steps):
        onset = i * arp_samples
        time_sec = onset / SAMPLE_RATE
        
        # Multi-Dimensional Arpeggiator Tiling Algorithm (Time, Pitch, Spatial/Amplitude, Timbre axes)
        t_tile = i % 4
        p_tile = (i // 4) % 3
        c_tile = (i // 24) % 4
        
        # Base pitch & initial level assignments per phase
        note_freq = arp_notes_l_sys[i % len(arp_notes_l_sys)] * 2.0
        lead_level = 0.0
        gate_dur = 0.12
        decay_const = 0.04
        dual_voice = False
        
        if time_sec < 90.0:
            # Complex technological arpeggiation in Phase 1
            lead_level = 0.14
            # Pitch Tiling Matrix: transpose to higher shimmering octaves
            pitch_matrix = [1.0, 1.5, 2.0]
            note_freq = note_freq * pitch_matrix[p_tile] * 2.0
            decay_const = 0.05
        elif time_sec < 180.0:
            lead_level = 0.15
        elif time_sec < 270.0:
            note_freq = note_freq * 2.0
            gate_dur = 0.24
            decay_const = 0.08
            lead_level = 0.18
        elif time_sec < 360.0:
            note_freq = note_freq * 2.0
            gate_dur = 0.16
            decay_const = 0.06
            dual_voice = True
            lead_level = 0.20
        elif time_sec < 450.0:
            # Phase 5: High-tech breakdown arpeggiation
            lead_level = 0.12
            # Pitch Tiling: subharmonic chord notes
            pitch_matrix = [0.5, 1.0, 1.5]
            note_freq = note_freq * pitch_matrix[p_tile]
            decay_const = 0.06
        elif time_sec < 540.0:
            slide_factor = 1.0 + 2.0 * ((time_sec - 450.0) / 90.0)
            note_freq = note_freq * slide_factor
            gate_dur = 0.10
            decay_const = 0.03
            lead_level = 0.15
        elif time_sec < 630.0:
            note_freq = note_freq * 2.0
            gate_dur = 0.16
            decay_const = 0.06
            dual_voice = True
            lead_level = 0.22
        else:
            slide_factor = 2.0 - ((time_sec - 630.0) / 90.0)
            note_freq = note_freq * slide_factor
            lead_level = 0.15 * (1.0 - (time_sec - 630.0) / 90.0)

        # Apply Spatial & Timbral axis tiling transformations
        if lead_level > 0.0:
            # Spatial tiling (amplitude gating LFO simulation for mono)
            spatial_mod = 0.85 + 0.15 * math.sin(time_sec * 2.0 * math.pi * 0.2)
            lead_level *= spatial_mod
            
            # Timbral tiling (decay rate shift over pattern matrices)
            timbral_decays = [0.7, 0.9, 1.1, 1.3]
            decay_const *= timbral_decays[c_tile]
            gate_dur = decay_const * 3.0
            
            # Syrinx Warble pitch modulation (rapid 18-24Hz bird-like pitch trill on vocal formants)
            if (180.0 <= time_sec < 270.0) or (540.0 <= time_sec < 630.0):
                warble_freq = 18.0 + 6.0 * math.sin(time_sec * 2.0 * math.pi * 0.1)
                warble = 1.0 + 0.08 * math.sin(2.0 * math.pi * warble_freq * (i * arp_samples / SAMPLE_RATE))
                note_freq *= warble
            
        if lead_level > 0.0:
            end_sample = min(num_samples, onset + int(gate_dur * SAMPLE_RATE))
            l_len = end_sample - onset
            if l_len > 0:
                l_age = np.arange(l_len) / SAMPLE_RATE
                lfo = 1.0 + 0.03 * np.sin(2.0 * np.pi * 6.0 * l_age)
                
                # Model selections based on active timeline phases
                if 90.0 <= time_sec < 180.0:
                    # 1. TUP-TUN relaxation oscillator simulation (rapid charge-discharge sawtooth)
                    phase1 = (note_freq * lfo * l_age) % 1.0
                    voice_mix = 1.0 - 2.0 * phase1
                elif 270.0 <= time_sec < 360.0:
                    # 2. Tunnel Diode chaotic oscillator model (non-linear foldback distortion)
                    phase1 = note_freq * lfo * l_age
                    raw_sine = np.sin(2.0 * np.pi * phase1)
                    voice_mix = np.sin(2.0 * np.pi * phase1 + 0.45 * raw_sine)
                else:
                    # Default: Sawtooth oscillators
                    phase1 = (note_freq * lfo * l_age) % 1.0
                    saw1 = 2.0 * phase1 - 1.0
                    if dual_voice:
                        phase2 = (note_freq * 1.012 * lfo * l_age) % 1.0
                        saw2 = 2.0 * phase2 - 1.0
                        voice_mix = 0.5 * saw1 + 0.5 * saw2
                    else:
                        voice_mix = saw1
                        
                # 3. Formant Filter Vowel Sweep (Phase 3 and Climax Phase 7)
                if (180.0 <= time_sec < 270.0) or (540.0 <= time_sec < 630.0):
                    # Peak bandpass filters at vowel resonance frequencies (F1/F2 formants)
                    formant_f = 600.0 + 400.0 * np.sin(2.0 * np.pi * 1.5 * l_age)
                    formant_res = np.sin(2.0 * np.pi * formant_f * l_age)
                    voice_mix = np.tanh(voice_mix * 1.5 + 0.5 * formant_res)
                    
                    # Vocal Echolocation clicks: rapid 4ms impulse clicks at 180Hz repetition rate
                    click_envelope = np.where((l_age * 180.0) % 1.0 < 0.12, 1.0, 0.0)
                    click_pulse = np.sin(2.0 * np.pi * 4200.0 * l_age) * click_envelope
                    voice_mix = voice_mix * 0.85 + click_pulse * 0.15
                    
                # 4. Ring Modulator Mode (Phase 3 and Phase 7)
                if (180.0 <= time_sec < 270.0) or (540.0 <= time_sec < 630.0):
                    modulator = np.sin(2.0 * np.pi * 370.0 * l_age)
                    voice_mix *= modulator
                    
                # 5. Stroboscope Rhythmic Volume Gating (Phase 5 breakdown)
                if 360.0 <= time_sec < 450.0:
                    strobe_env = np.where((l_age * 12.0) % 1.0 < 0.40, 1.0, 0.05)
                    voice_mix *= strobe_env
                    
                # 6. Push-Pull Class AB Amplifier Simulation (Phase 6 and Climax Phase 7)
                if (450.0 <= time_sec < 630.0):
                    # Saturation plus crossover distortion centered at zero
                    pp_sat = np.tanh(voice_mix * 2.2)
                    voice_mix = np.where(np.abs(pp_sat) < 0.05, pp_sat * 0.35, pp_sat)
                    
                lead[onset:end_sample] += voice_mix * np.exp(-l_age / decay_const) * lead_level

    # Feedback Delay / Echo Line on Lead Channel
    delay_samples = int(beat_samples * 0.75)
    feedback = 0.45
    delay_buffer = np.zeros_like(lead)
    for s in range(len(lead)):
        if s >= delay_samples:
            delay_buffer[s] = lead[s - delay_samples] + delay_buffer[s - delay_samples] * feedback
    lead += delay_buffer * 0.45

    # -------------------------------------------------------------
    # 4. Technology 3: Spectral IFFT Synthesis (Ch 4 Ambient Pad)
    # -------------------------------------------------------------
    # We synthesize the harmonic chord backing directly in the frequency domain
    pad = np.zeros_like(t)
    chord_duration = beat_samples * 8
    num_chords = int(num_samples / chord_duration)
    chords = [
        [110.0, 130.81, 164.81], # Am
        [87.31, 110.0, 130.81],  # F
        [65.41, 82.41, 98.00],   # C
        [73.42, 92.50, 110.00]   # G
    ]
    
    block_size = 2048
    fft_len = block_size // 2 + 1
    freq_resolution = SAMPLE_RATE / block_size
    
    for c_idx in range(num_chords):
        onset = c_idx * chord_duration
        chord_freqs = chords[c_idx % len(chords)]
        end_sample = min(num_samples, onset + chord_duration)
        c_len = end_sample - onset
        if c_len > 0:
            num_blocks = int(np.ceil(c_len / block_size))
            chord_pad_block = np.zeros(num_blocks * block_size)
            
            for b in range(num_blocks):
                spectrum = np.zeros(fft_len, dtype=complex)
                for freq in chord_freqs:
                    bin_idx = int(np.round(freq / freq_resolution))
                    if 0 < bin_idx < fft_len:
                        # Fundamental
                        spectrum[bin_idx] = 1.0 * np.exp(1j * np.random.uniform(0, 2.0 * np.pi))
                        # Detuned 2nd harmonic
                        if bin_idx * 2 < fft_len:
                            spectrum[bin_idx * 2] = 0.55 * np.exp(1j * np.random.uniform(0, 2.0 * np.pi))
                            
                # Perform spectral IFFT
                chord_pad_block[b * block_size : (b + 1) * block_size] = np.fft.irfft(spectrum, n=block_size)
                
            # Apply slow attack-release envelopes
            c_age = np.arange(c_len) / SAMPLE_RATE
            dur = chord_duration / SAMPLE_RATE
            env = np.ones_like(c_age)
            attack_len = min(1.8, dur * 0.35)
            release_len = min(1.8, dur * 0.35)
            env[c_age < attack_len] = c_age[c_age < attack_len] / attack_len
            env[c_age > (dur - release_len)] = (dur - c_age[c_age > (dur - release_len)]) / release_len
            
            pad_segment = chord_pad_block[:c_len] * env * 0.16
            
            # Philicorda Organ additive synthesis: active from Phase 2 (90s onwards)
            if onset / SAMPLE_RATE >= 90.0:
                organ_segment = np.zeros(c_len)
                for freq in chord_freqs:
                    organ_segment += np.sin(2.0 * np.pi * freq * c_age) * 0.40
                    organ_segment += np.sin(2.0 * np.pi * (freq * 0.5) * c_age) * 0.30
                    organ_segment += np.sin(2.0 * np.pi * (freq * 2.0) * c_age) * 0.25
                    organ_segment += np.sin(2.0 * np.pi * (freq * 3.0) * c_age) * 0.15
                pad_segment += np.tanh(organ_segment * 0.5) * env * 0.08
                
            pad[onset:end_sample] += pad_segment

    # -------------------------------------------------------------
    # 5. Channel 5: Auncient Bird Whistles & Crow Caws
    # -------------------------------------------------------------
    whistles = np.zeros_like(t)
    whistle_samples = int(beat_samples * 8)
    num_whistle_steps = int(num_samples / whistle_samples)
    
    for i in range(num_whistle_steps):
        onset = i * whistle_samples
        time_sec = onset / SAMPLE_RATE
        
        # Bird whistle chirps
        whistle_trigger = False
        if time_sec < 90.0 and i % 2 == 0:
            whistle_trigger = True
        elif 360.0 <= time_sec < 450.0 and i % 3 == 0:
            whistle_trigger = True
        elif time_sec >= 630.0 and i % 2 == 0:
            whistle_trigger = True
            
        if whistle_trigger:
            motif_len = int(SAMPLE_RATE * 2.0)
            end_sample = min(num_samples, onset + motif_len)
            w_len = end_sample - onset
            if w_len > 0:
                w_age = np.arange(w_len) / SAMPLE_RATE
                w_signal = np.zeros(w_len)
                for chirp_start in [0.0, 0.4, 0.8]:
                    c_onset = int(chirp_start * SAMPLE_RATE)
                    c_dur = int(0.20 * SAMPLE_RATE)
                    c_end = min(w_len, c_onset + c_dur)
                    c_len = c_end - c_onset
                    if c_len > 0:
                        c_age = np.arange(c_len) / SAMPLE_RATE
                        freq = 2400.0 + 1400.0 * (1.0 - np.exp(-c_age / 0.04))
                        c_phase = 2.0 * np.pi * np.cumsum(freq) / SAMPLE_RATE
                        c_env = np.exp(-c_age / 0.05) * np.sin(c_phase)
                        
                        # Formant vocal modulation: morph from EE to OH over chirp
                        ratio = c_age / 0.20
                        f1 = 270.0 * (1.0 - ratio) + 570.0 * ratio
                        f2 = 2290.0 * (1.0 - ratio) + 840.0 * ratio
                        f3 = 3010.0 * (1.0 - ratio) + 2410.0 * ratio
                        formant_mod = 0.4 * np.sin(2.0 * np.pi * f1 * c_age) + \
                                      0.35 * np.sin(2.0 * np.pi * f2 * c_age) + \
                                      0.25 * np.sin(2.0 * np.pi * f3 * c_age)
                        c_env = c_env * (0.3 + 0.7 * formant_mod)
                        w_signal[c_onset:c_end] += c_env * 0.35
                whistles[onset:end_sample] += w_signal
                
        # Crow Caw bird calls
        caw_trigger = False
        if time_sec < 90.0 and i % 4 == 1:
            caw_trigger = True
        elif 360.0 <= time_sec < 450.0 and i % 5 == 2:
            caw_trigger = True
        elif time_sec >= 630.0 and i % 4 == 3:
            caw_trigger = True
            
        if caw_trigger:
            caw_len = int(SAMPLE_RATE * 0.45)
            end_sample = min(num_samples, onset + caw_len)
            cw_len = end_sample - onset
            if cw_len > 0:
                cw_age = np.arange(cw_len) / SAMPLE_RATE
                f_base = 240.0 - 40.0 * (cw_age / 0.45)
                ring_mod = np.sin(2.0 * np.pi * 35.0 * cw_age)
                cw_phase = 2.0 * np.pi * np.cumsum(f_base) / SAMPLE_RATE
                osc = np.sin(cw_phase) * 0.6 + np.sin(cw_phase * 2.0) * 0.3
                harsh_osc = osc * (0.7 + 0.3 * ring_mod)
                formant = np.sin(2.0 * np.pi * 1000.0 * cw_age) * 0.2
                caw_val = np.tanh((harsh_osc + formant) * 2.2)
                
                # Formant vocal modulation: morph from AH to OO over caw
                ratio = cw_age / 0.45
                f1 = 730.0 * (1.0 - ratio) + 300.0 * ratio
                f2 = 1090.0 * (1.0 - ratio) + 870.0 * ratio
                f3 = 2440.0 * (1.0 - ratio) + 2240.0 * ratio
                formant_mod = 0.4 * np.sin(2.0 * np.pi * f1 * cw_age) + \
                              0.35 * np.sin(2.0 * np.pi * f2 * cw_age) + \
                              0.25 * np.sin(2.0 * np.pi * f3 * cw_age)
                caw_val = caw_val * (0.3 + 0.7 * formant_mod)
                
                shaky_env = np.exp(-cw_age / 0.12) * (0.8 + 0.2 * np.sin(2.0 * np.pi * 14.0 * cw_age))
                whistles[onset:end_sample] += caw_val * shaky_env * 0.40

    # Mix the five channels
    mix = (drums * 0.32) + (bass * 0.26) + (lead * 0.18) + (pad * 0.14) + (whistles * 0.10)
    mix = np.clip(mix, -1.0, 1.0)
    
    # -------------------------------------------------------------
    # Decay Tail Wrapping: Seamlessly fold overspill back to start
    # -------------------------------------------------------------
    loop_samples = int(SAMPLE_RATE * DURATION)
    tail = mix[loop_samples:]
    mix[:len(tail)] += tail
    mix = mix[:loop_samples]
    
    backing_path = "temp_chaos_extended_loop.wav"
    byte_data = bytearray(44 + len(mix) * 2)
    byte_data[0:4] = b"RIFF"
    byte_data[4:8] = int(36 + len(mix) * 2).to_bytes(4, "little")
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
    byte_data[40:44] = int(len(mix) * 2).to_bytes(4, "little")
    
    offset = 44
    for val in mix:
        s = max(-1.0, min(1.0, val))
        v = int(s * 32767) if s >= 0 else int(s * 32768)
        byte_data[offset:offset+2] = v.to_bytes(2, "little", signed=True)
        offset += 2
        
    with open(backing_path, "wb") as f:
        f.write(byte_data)
    return backing_path

def rotate4D(x, y, z, w, theta, phi):
    cos_t, sin_t = math.cos(theta), math.sin(theta)
    rx = x * cos_t - w * sin_t
    rw = x * sin_t + w * cos_t
    
    cos_p, sin_p = math.cos(phi), math.sin(phi)
    ry = y * cos_p - z * sin_p
    rz = y * sin_p + z * cos_p
    
    return rx, ry, rz, rw

def generate_video_direct_extended(audio_path, output_mp4):
    print("[TSFI] Initiating direct FFmpeg pipe rendering (Vertical 720x1280)...")
    
    cmd = (
        f"ffmpeg -y -f rawvideo -vcodec rawvideo -s 720x1280 -pix_fmt rgb24 -r {FPS} -i - "
        f"-i {audio_path} -c:v libx264 -pix_fmt yuv420p -preset fast "
        f"-c:a aac -b:a 192k -t {DURATION} {output_mp4}"
    )
    
    pipe = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE)
    total_frames = int(DURATION * FPS)
    
    # Initialize 3D Starfield particles
    np.random.seed(420)
    stars = []
    for _ in range(80):
        stars.append({
            'x': np.random.uniform(-300, 300),
            'y': np.random.uniform(-400, 400),
            'z_init': np.random.uniform(20, 600)
        })
        
    # Define Tesseract 16 vertices in 4D space
    vertices_4d = []
    for x in [-1, 1]:
        for y in [-1, 1]:
            for z in [-1, 1]:
                for w in [-1, 1]:
                    vertices_4d.append((x, y, z, w))
                    
    edges = []
    for i in range(16):
        for j in range(i + 1, 16):
            diff = 0
            for k in range(4):
                if vertices_4d[i][k] != vertices_4d[j][k]:
                    diff += 1
            if diff == 1:
                edges.append((i, j))

    # Define 3D Torus vertices and edges orbiting the tesseract
    torus_vertices = []
    R_torus = 140.0
    r_torus = 16.0
    num_theta = 8
    num_phi = 16
    for p in range(num_phi):
        phi_val = (p / num_phi) * 2.0 * math.pi
        for t in range(num_theta):
            theta_val = (t / num_theta) * 2.0 * math.pi
            tx = (R_torus + r_torus * math.cos(theta_val)) * math.cos(phi_val)
            ty = r_torus * math.sin(theta_val)
            tz = (R_torus + r_torus * math.cos(theta_val)) * math.sin(phi_val)
            torus_vertices.append((tx, ty, tz))
            
    torus_edges = []
    for p in range(num_phi):
        for t in range(num_theta):
            curr = p * num_theta + t
            next_t = p * num_theta + ((t + 1) % num_theta)
            torus_edges.append((curr, next_t))
            next_p = ((p + 1) % num_phi) * num_theta + t
            torus_edges.append((curr, next_p))

    # Precompute Lorenz chaotic attractor trajectory
    lorenz_pts = []
    lx, ly, lz = 0.1, 0.0, 0.0
    dt_lorenz = 0.008
    for _ in range(6000):
        dx = 10.0 * (ly - lx) * dt_lorenz
        dy = (lx * (28.0 - lz) - ly) * dt_lorenz
        dz = (lx * ly - (8.0 / 3.0) * lz) * dt_lorenz
        lx += dx
        ly += dy
        lz += dz
        lorenz_pts.append((lx * 4.0, (lz - 25.0) * 4.0, ly * 4.0))

    # Initialize Verlet soft-body FET discharge grid (Rule 10: Verlet constraints apply to FET discharge)
    fet_grid = []
    for r in range(4):
        for c in range(4):
            fet_grid.append({
                'x': (c - 1.5) * 15.0,
                'y': (r - 1.5) * 15.0,
                'z': 0.0,
                'px': (c - 1.5) * 15.0,
                'py': (r - 1.5) * 15.0,
                'pz': 0.0
            })

    # Parse USD scene for materials (General research topic)
    import re
    usd_color = (120, 79, 53) # Default fur color fallback
    usd_roughness = 0.95
    try:
        with open("teddy_walk_scene.usda", "r") as f_usd:
            content = f_usd.read()
            color_match = re.search(r"color3f inputs:diffuseColor\s*=\s*\(([^)]+)\)", content)
            if color_match:
                rgb_vals = [float(x.strip()) for x in color_match.group(1).split(",")]
                usd_color = (int(rgb_vals[0]*255), int(rgb_vals[1]*255), int(rgb_vals[2]*255))
            rough_match = re.search(r"float inputs:roughness\s*=\s*([0-9.]+)", content)
            if rough_match:
                usd_roughness = float(rough_match.group(1))
    except Exception:
        pass

    CX = 360
    CY = 560

    for frame in range(total_frames):
        time = frame / float(FPS)
        progress = time / DURATION
        
        # 1. Retro trigonometric plasma background (fixed-bound looping)
        px, py = np.meshgrid(np.linspace(0, 4.5, 90), np.linspace(0, 8.0, 160))
        time_angle = progress * 2.0 * math.pi * 6.0
        plasma_val = (
            np.sin(px + time_angle) +
            np.sin(1.5 * (py - time_angle)) +
            np.sin(px + py + time_angle) +
            np.cos(np.sqrt(px**2 + py**2) - time_angle)
        )
        plasma_val = np.clip((plasma_val + 4.0) / 8.0, 0.0, 1.0)
        
        pr = (plasma_val * 35.0).astype(np.uint8)
        pg = (plasma_val * 15.0).astype(np.uint8)
        pb = (40.0 + plasma_val * 70.0).astype(np.uint8)
        
        plasma_arr = np.stack([pr, pg, pb], axis=-1)
        img = Image.fromarray(plasma_arr).resize((720, 1280), Image.Resampling.BILINEAR)
        draw = ImageDraw.Draw(img)
        
        beat_phase = (time / BEAT_DUR) * math.pi * 2.0
        beat_amplitude = max(0.0, math.sin(beat_phase)) * math.exp(-((time % BEAT_DUR) / 0.15))

        # 2. Draw Amiga copper raster bars (Looping)
        for bar_idx in range(6):
            y_base1 = 560.0 + 350.0 * math.sin(progress * 2.0 * math.pi * 24.0 + bar_idx * 0.8)
            y_base2 = 560.0 + 280.0 * math.cos(progress * 2.0 * math.pi * 48.0 - bar_idx * 0.5)
            bar_y = (y_base1 + y_base2) / 2.0
            
            bar_height = 50
            for dy in range(-bar_height // 2, bar_height // 2):
                y_pos = int(bar_y + dy)
                if 0 <= y_pos < 1280:
                    intensity = 1.0 - abs(dy) / (bar_height / 2.0)
                    r = int(10 * intensity)
                    g = int(24 * intensity + 20 * intensity * math.sin(progress * 2.0 * math.pi * 96.0))
                    b = int(120 * intensity + 60 * intensity * math.cos(progress * 2.0 * math.pi * 48.0 + bar_idx))
                    draw.line([(0, y_pos), (720, y_pos)], fill=(r, g, b), width=1)

        # 3. Looping 3D Parallax Starfield (Speed varies by phase)
        if time < 90.0:
            star_speed = 4.0 + 8.0 * beat_amplitude
        elif time < 180.0:
            star_speed = 8.0
        elif time < 270.0:
            star_speed = 8.0 + 18.0 * beat_amplitude
        elif time < 360.0:
            star_speed = 12.0 + 25.0 * beat_amplitude
        elif time < 450.0:
            star_speed = 3.0 + 6.0 * beat_amplitude
        elif time < 540.0:
            star_speed = 3.0 + 15.0 * ((time - 450.0) / 90.0)
        elif time < 630.0:
            star_speed = 14.0 + 28.0 * beat_amplitude
        else:
            star_speed = 8.0 * (1.0 - (time - 630.0)/90.0)
            
        for star in stars:
            z = (star['z_init'] - frame * (star_speed / 8.0) * 10.0) % 580 + 20
            factor = 280.0 / z
            sx = CX + int(star['x'] * factor)
            sy = CY + int(star['y'] * factor)
            s_size = max(1, int((2.0 + 4.0 * beat_amplitude) * factor))
            
            if 0 <= sx < 720 and 0 <= sy < 1280:
                glow_val = min(255, int((180 + 75 * beat_amplitude) * factor))
                draw.ellipse([sx, sy, sx + s_size, sy + s_size], fill=(glow_val, glow_val, 255))

        # 4. Looping 3D Perspective Grid Floor (Horizon Y=850 to boundary Y=1150)
        draw.line([(0, 850), (720, 850)], fill="#002244", width=2)
        if time < 90.0:
            grid_speed = 24.0 / DURATION
        elif time < 360.0:
            grid_speed = 72.0 / DURATION
        elif time < 450.0:
            grid_speed = 18.0 / DURATION
        elif time < 630.0:
            grid_speed = 72.0 / DURATION
        else:
            grid_speed = 12.0 / DURATION
        num_horizontal = 12
        for i in range(num_horizontal):
            ratio = ((i - time * grid_speed) % num_horizontal) / num_horizontal
            y_grid = 850.0 + 300.0 * (ratio ** 2)
            intensity = int(20 + 180 * ratio)
            draw.line([(0, y_grid), (720, y_grid)], fill=(0, intensity // 3, intensity), width=1)
            
        num_vertical = 16
        for i in range(num_vertical + 1):
            x_bottom = (i / num_vertical) * 1120.0 - 200.0
            draw.line([(CX, 850), (x_bottom, 1150)], fill=(0, 40, 110), width=1)

        # 5. Equalizer Spectrum Bars (8 bands vertically stacked along side margins)
        num_bands = 8
        bar_w = 12
        bar_spacing = 4
        for band_idx in range(num_bands):
            band_val = beat_amplitude * 0.4 + 0.6 * (0.5 + 0.5 * math.sin(progress * 2.0 * math.pi * 96.0 * (2.0 + band_idx) + band_idx))
            
            num_segs = int(np.clip(band_val, 0.0, 1.0) * 12)
            lx = 30 + band_idx * (bar_w + bar_spacing)
            rx = 690 - (num_bands - 1 - band_idx) * (bar_w + bar_spacing)
            
            for seg in range(12):
                y_pos = 780 - seg * 12
                if seg < 6:
                    color = (0, 220, 80)
                elif seg < 10:
                    color = (220, 220, 0)
                else:
                    color = (255, 40, 40)
                    
                if seg < num_segs:
                    draw.rectangle([lx, y_pos, lx + bar_w, y_pos + 8], fill=color)
                    draw.rectangle([rx, y_pos, rx + bar_w, y_pos + 8], fill=color)
                else:
                    draw.rectangle([lx, y_pos, lx + bar_w, y_pos + 8], outline=(30, 30, 45))
                    draw.rectangle([rx, y_pos, rx + bar_w, y_pos + 8], outline=(30, 30, 45))

        # 6. Draw dynamic FFT visualizer line at the bottom
        wave_pts = []
        amp = 20.0 + 90.0 * beat_amplitude
        for x in range(0, 720, 10):
            y = 1170.0 + amp * math.sin(x * 0.03 + progress * 2.0 * math.pi * 2088.0) * math.cos(x * 0.01)
            wave_pts.append((x, y))
        draw.line(wave_pts, fill="#ff007f", width=2)

        # 7. 4D Tesseract projection
        theta = progress * 2.0 * math.pi * 96.0
        phi = progress * 2.0 * math.pi * 48.0
        pulse = 110.0 + 40.0 * beat_amplitude
        
        projected = []
        for pt in vertices_4d:
            rx, ry, rz, rw = rotate4D(pt[0], pt[1], pt[2], pt[3], theta, phi)
            d = 2.5
            factor = 1.0 / (d - rw)
            x3d = rx * factor * pulse
            y3d = ry * factor * pulse
            z3d = rz * factor * pulse
            
            proj_x = CX + x3d
            proj_y = CY + y3d
            projected.append((proj_x, proj_y))
            
        hue_tess = int((progress * 360 * 24 + beat_amplitude * 120) % 360)
        line_color = f"hsl({hue_tess}, 100%, 65%)"
        line_width = 2 + int(3 * beat_amplitude)
        
        for edge in edges:
            p1 = projected[edge[0]]
            p2 = projected[edge[1]]
            draw.line([p1, p2], fill=line_color, width=line_width)
            
        for pt in projected:
            draw.ellipse([pt[0]-4, pt[1]-4, pt[0]+4, pt[1]+4], fill="#ff007f", outline="#ffffff", width=1)

        # 8. Orbiting 3D Torus
        if time < 690.0:
            t_rot_x = progress * 2.0 * math.pi * 72.0
            t_rot_y = progress * 2.0 * math.pi * 96.0
            cos_tx, sin_tx = math.cos(t_rot_x), math.sin(t_rot_x)
            cos_ty, sin_ty = math.cos(t_rot_y), math.sin(t_rot_y)
            
            torus_projected = []
            for vx, vy, vz in torus_vertices:
                ry = vy * cos_tx - vz * sin_tx
                rz = vy * sin_tx + vz * cos_tx
                rx = vx * cos_ty - rz * sin_ty
                rz = vx * sin_ty + rz * cos_ty
                
                d_cam = 450.0
                factor = 280.0 / (d_cam - rz)
                proj_x = CX + rx * factor
                proj_y = CY + ry * factor
                torus_projected.append((proj_x, proj_y))
                
            for edge in torus_edges:
                p1 = torus_projected[edge[0]]
                p2 = torus_projected[edge[1]]
                draw.line([p1, p2], fill="#00f2fe", width=1)

        # 9. Lorenz Attractor (Active in drops and climax)
        show_attractor = (180.0 <= time < 360.0) or (540.0 <= time < 630.0)
        if show_attractor:
            trail_len = 150
            trail_step = 4
            trail_end = (frame * trail_step) % len(lorenz_pts)
            trail_start = max(0, trail_end - trail_len)
            trail = lorenz_pts[trail_start:trail_end]
            
            ang_la = progress * 2.0 * math.pi * 48.0
            cos_la, sin_la = math.cos(ang_la), math.sin(ang_la)
            la_projected = []
            for lx, ly, lz in trail:
                rx = lx * cos_la - lz * sin_la
                rz = lx * sin_la + lz * cos_la
                ry = ly
                
                d_cam = 450.0
                factor = 280.0 / (d_cam - rz)
                proj_x = CX + int(rx * factor)
                proj_y = CY + int(ry * factor)
                la_projected.append((proj_x, proj_y))
                
            for j in range(1, len(la_projected)):
                p1 = la_projected[j - 1]
                p2 = la_projected[j]
                fade = j / len(la_projected)
                color = (0, int(150 * fade + 105 * beat_amplitude), int(50 * fade))
                draw.line([p1, p2], fill=color, width=2)

        # 10. Draw Demoscene Bouncing Block Text "TSFi/2"
        text_to_draw = "TSFi/2"
        block_size = 5
        spacing = 2
        
        orbit_angle = progress * 2.0 * math.pi * 12.0
        cos_o, sin_o = math.cos(orbit_angle), math.sin(orbit_angle)
        
        for char_idx, char in enumerate(text_to_draw):
            if char in BLOCK_FONT:
                bounce_offset = math.sin(progress * 2.0 * math.pi * BEAT_LOOP_COUNT - char_idx * 0.9) * 12.0
                grid = BLOCK_FONT[char]
                
                cx = (char_idx - 2.5) * 45.0
                cy = -230.0
                
                for r in range(5):
                    for c in range(5):
                        if grid[r][c] == 1:
                            lx = cx + (c - 2) * (block_size + spacing)
                            ly = cy + (r - 2) * (block_size + spacing) + bounce_offset
                            lz = 0.0
                            
                            rx = lx * cos_o - lz * sin_o
                            rz = lx * sin_o + lz * cos_o
                            ry = ly
                            
                            d_cam = 400.0
                            factor = 280.0 / (d_cam - rz)
                            
                            bx = CX + rx * factor
                            by = CY + ry * factor
                            size_proj = max(1, int(block_size * factor))
                            
                            hue = int((progress * 360 * 96 + char_idx * 40 + r * 15) % 360)
                            color_str = f"hsl({hue}, 100%, 65%)"
                            draw.rectangle([bx, by, bx + size_proj, by + size_proj], fill=color_str)

        # 9b. Update and Draw Verlet soft-body FET grid (Rule 10: soft body physics applies only to FET discharge cycles)
        grid_rot_y = time * 2.2
        cos_gr, sin_gr = math.cos(grid_rot_y), math.sin(grid_rot_y)
        
        # Apply Verlet step to node coordinates reacting to beat_amplitude
        for node in fet_grid:
            temp_x, temp_y, temp_z = node['x'], node['y'], node['z']
            
            # Simple spring constraint pull towards origin plus beat oscillation force
            accel_z = -0.1 * node['z'] + beat_amplitude * 45.0 * math.sin(time * 16.0 + node['x'] * 0.1)
            accel_x = -0.05 * node['x']
            accel_y = -0.05 * node['y']
            
            # Verlet integration
            node['x'] = 2.0 * node['x'] - node['px'] + accel_x * 0.001
            node['y'] = 2.0 * node['y'] - node['py'] + accel_y * 0.001
            node['z'] = 2.0 * node['z'] - node['pz'] + accel_z * 0.001
            
            node['px'], node['py'], node['pz'] = temp_x, temp_y, temp_z

        # Project and draw grid connections
        gr_projected = []
        for node in fet_grid:
            sx, sy, sz = node['x'] * 1.5, node['y'] * 1.5, node['z'] * 1.5
            rx = sx * cos_gr - sz * sin_gr
            rz = sx * sin_gr + sz * cos_gr
            ry = sy
            
            d_cam = 450.0
            factor = 280.0 / (d_cam - rz)
            proj_x = 180 + int(rx * factor) # Left margin coordinate shift
            proj_y = 1000 + int(ry * factor)
            gr_projected.append((proj_x, proj_y))

        # Draw grid wireframe connections
        for r in range(4):
            for c in range(4):
                curr_idx = r * 4 + c
                p_curr = gr_projected[curr_idx]
                if c < 3:
                    p_right = gr_projected[curr_idx + 1]
                    draw.line([p_curr, p_right], fill="#c5a059", width=1)
                if r < 3:
                    p_down = gr_projected[curr_idx + 4]
                    draw.line([p_curr, p_down], fill="#c5a059", width=1)

        # 9c. Draw Federal Worker active phase progress indicator on left margin
        phases_names = ["Imposition", "Qualified", "Audited", "Confirmed", "Selected"]
        current_phase_idx = int((time / DURATION) * 5) % 5
        for p_idx, p_name in enumerate(phases_names):
            py_pos = 140 + p_idx * 30
            is_active = (p_idx == current_phase_idx)
            is_past = (p_idx < current_phase_idx)
            
            circle_color = "#c5a059" if (is_active or is_past) else "#332211"
            draw.ellipse([40, py_pos, 50, py_pos + 10], fill=circle_color)
            text_color = "#e6dfd3" if is_active else "#665544"
            draw.text((60, py_pos - 2), p_name, fill=text_color)

        # 9d. Draw USD walk-and-wave coordinate trajectory (syncing with frame codes 0-719)
        walk_pts = []
        for step in range(max(0, frame - 120), frame):
            walk_x = CX + int(120.0 * math.sin(step * 0.03))
            walk_y = CY + 180 + int(60.0 * math.cos(step * 0.06) * math.sin(step * 0.01))
            walk_pts.append((walk_x, walk_y))
        if len(walk_pts) > 1:
            draw.line(walk_pts, fill=usd_color, width=2)

        # Telemetry Display in Vaesen Gothic styling colors (#c5a059 gold, #e6dfd3 cream)
        draw.text((40, 60), "TSFi/2: PROGRESSIVE SET", fill="#c5a059")
        draw.text((40, 80), f"PHASE: {int(time / 90.0) + 1}/8", fill="#e6dfd3")
        draw.text((40, 100), f"TIME: {int(time)}s / 720s", fill="#e6dfd3")
        
        # WinchesterMQ registers & Federal Worker simulation telemetry
        draw.text((450, 60), f"AuncientAnalyzer: Permissible (Opcodes: A,S) | DEFCON: 5", fill="#e6dfd3")
        draw.text((450, 80), "Worker: PHASE_SELECTED_WORKER (Saat: 1M)", fill="#c5a059")
        draw.text((450, 100), f"USD Scene: teddy_walk_scene.usda | diffuseColor: {usd_color}", fill="#c5a059")
        
        # Amiga Scrolltext at the bottom
        scroll_text = "TSFi/2 AUNCIENT WINCHESTERMQ HARDWARE DECK EMULATION --- LOOPING EXTENDED 12-MINUTE DEMO PROGRESSION --- 174 BPM DRUM AND BASS --- PEGGING THE BLOCKCHAIN FOREVER..."
        char_spacing = 15
        total_text_width = len(scroll_text) * char_spacing
        scroll_x = 720 - int(progress * 15.0 * (total_text_width + 720)) % (total_text_width + 720)
        draw.text((scroll_x, 1220), scroll_text, fill="#c5a059")

        # Directly pipe RGB raw bytes to FFmpeg
        pipe.stdin.write(img.tobytes())
        
        if frame % 300 == 0:
            print(f"  -> Rendering Extended: {frame}/{total_frames} frames ({progress*100:.1f}%)...")
            
    pipe.stdin.close()
    pipe.wait()
    print(f"[SUCCESS] Extended loop successfully saved to {output_mp4}!")

def cleanup():
    print("[CLEANUP] Purging temporary audio track...")
    try:
        os.remove("temp_chaos_extended_loop.wav")
    except Exception:
        pass

if __name__ == "__main__":
    backing = generate_chaos_audio_extended()
    
    # Destination in active conversation artifacts directory
    output_dir = "/home/mariarahel/.gemini/antigravity-cli/brain/1b5b5d7d-d753-41a3-9301-e621c99e1f05"
    output_mp4 = os.path.join(output_dir, "chaos_of_sidrath_extended.mp4")
    
    generate_video_direct_extended(backing, output_mp4)
    cleanup()
