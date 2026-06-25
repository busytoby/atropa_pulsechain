import time
import sys
import numpy as np
from scripts.bionika_syrinx_speech import generate_syrinx_speech

def benchmark_speech_performance():
    phrases = [
        "short phrase",
        "auncient bionika speaks with the syrinx",
        "the lobes are formed ephemerally and spontaneously as accumulators by the physical collisions of the membrane oscillations"
    ]
    
    print("=== TSFI2 BIONIKA SYRINX SPEECH DSP BENCHMARK ===")
    print(f"{'Phrase Length (Chars)':<25} | {'Audio Duration (s)':<18} | {'Compute Time (s)':<18} | {'Real-Time Factor (RTF)':<20}")
    print("-" * 90)
    
    for phrase in phrases:
        # Measure duration of audio (0.2 seconds per phoneme-ish)
        # Note: generate_syrinx_speech determines duration by resolved phonemes
        start_time = time.perf_counter()
        generate_syrinx_speech(phrase, "tmp_benchmark.wav")
        end_time = time.perf_counter()
        
        compute_time = end_time - start_time
        
        # Read duration of generated audio file
        import wave
        with wave.open("tmp_benchmark.wav", "rb") as wav_file:
            frames = wav_file.getnframes()
            rate = wav_file.getframerate()
            duration = frames / float(rate)
            
        rtf = duration / compute_time
        print(f"{len(phrase):<25} | {duration:<18.4f} | {compute_time:<18.6f} | {rtf:<20.2f}x")
        
    # Clean up
    import os
    try:
        os.remove("tmp_benchmark.wav")
    except Exception:
        pass

if __name__ == "__main__":
    benchmark_speech_performance()
