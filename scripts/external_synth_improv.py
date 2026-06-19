import urllib.request
import json
import time

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
        print(f"Error sending note: {e}")

def play_prophecy_improvisation():
    print("[PYTHON 3RD-PARTY CLIENT] Initiating NEW complex arpeggiation/chords sequence...")
    
    # We will trigger three chords and run a quick cascading melody to rotate the quaternion visualizer
    chords = [
        # Chord 1: Minor triad (A3, C4, E4)
        [("A3", 220.00), ("C4", 261.63), ("E4", 329.63)],
        # Chord 2: Major triad (F3, A3, C4)
        [("F3", 174.61), ("A3", 220.00), ("C4", 261.63)],
        # Chord 3: Major triad (G3, B3, D4)
        [("G3", 196.00), ("B3", 246.94), ("D4", 293.66)]
    ]
    
    for chord in chords:
        print(f"[PYTHON CLIENT] Playing Chord: {', '.join([n[0] for n in chord])}")
        # Note ON for all notes in the chord
        for note, freq in chord:
            send_note(note, freq, 1)
        time.sleep(0.4)
        
        # Note OFF for all notes in the chord
        for note, freq in chord:
            send_note(note, freq, 0)
        time.sleep(0.1)

    # Cascading high melody
    melody = [
        ("E5", 659.25), ("D5", 587.33), ("C5", 523.25), ("B4", 493.88), 
        ("A4", 440.00), ("G4", 392.00), ("F4", 349.23), ("E4", 329.63)
    ]
    
    for note, freq in melody:
        print(f"[PYTHON CLIENT] Melody Note: {note} ({freq} Hz)")
        send_note(note, freq, 1)
        time.sleep(0.18)
        send_note(note, freq, 0)
        time.sleep(0.02)

if __name__ == "__main__":
    play_prophecy_improvisation()
