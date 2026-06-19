import urllib.request
import json
import time
import math

def send_note(note, freq, control):
    url = "http://127.0.0.1:3000/api/synth-feed?sessionId=global"
    data = json.dumps({
        "type": "synth_poke",
        "note": note,
        "freq": freq,
        "control": control
    }).encode("utf-8")
    
    req = urllib.request.Request(
        url, 
        data=data, 
        headers={"Content-Type": "application/json"}
    )
    
    try:
        with urllib.request.urlopen(req) as response:
            response.read()
    except Exception as e:
        pass

def play_evolving_performance():
    print("[PYTHON PERFORMANCE] Starting a 30-second evolving synth performance...")
    
    # Define note names and frequencies for a wide Auncient modal scale
    scale = [
        ("A3", 220.00), ("B3", 246.94), ("C4", 261.63), ("D4", 293.66),
        ("E4", 329.63), ("F4", 349.23), ("G4", 392.00), ("A4", 440.00),
        ("B4", 493.88), ("C5", 523.25), ("D5", 587.33), ("E5", 659.25),
        ("F5", 698.46), ("G5", 783.99), ("A5", 880.00)
    ]
    
    # We will loop for 60 ticks (60 * 0.5s = 30 seconds)
    for tick in range(60):
        # We will dynamically generate chords and melodies based on mathematical functions
        is_chord_tick = (tick % 6 == 0)
        
        if is_chord_tick:
            # Play a shifting triad chord based on the tick index
            base_idx = (tick // 6) % 5
            chord_notes = [scale[base_idx], scale[base_idx + 2], scale[base_idx + 4]]
            print(f"[PERFORMANCE] Shifting Chord Triad: {', '.join([n[0] for n in chord_notes])}")
            
            for note, freq in chord_notes:
                send_note(note, freq, 1)
            time.sleep(0.3)
            for note, freq in chord_notes:
                send_note(note, freq, 0)
            time.sleep(0.2)
        else:
            # Play an arpeggiating melody note that dances around the scale
            phase = tick * 0.4
            idx = int(7 + 5 * math.sin(phase) + 2 * math.cos(phase * 0.5))
            idx = max(0, min(idx, len(scale) - 1))
            note, freq = scale[idx]
            
            print(f"[PERFORMANCE] Melody Note: {note} ({freq:.2f} Hz)")
            send_note(note, freq, 1)
            time.sleep(0.4)
            send_note(note, freq, 0)
            time.sleep(0.1)

    print("[PERFORMANCE] Completed the 30-second set.")

if __name__ == "__main__":
    play_evolving_performance()
