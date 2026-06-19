import urllib.request
import json
import time
import sys

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

def play_arpeggio():
    print("[PYTHON 3RD-PARTY CLIENT] Initiating arpeggiator control loop via Python...")
    # Play a sequence of notes to demonstrate external script control
    melody = [
        ("C4", 261.63), ("D4", 293.66), ("E4", 329.63), ("F4", 349.23),
        ("G4", 392.00), ("A4", 440.00), ("B4", 493.88), ("C5", 523.25)
    ]
    
    for note, freq in melody:
        print(f"[PYTHON CLIENT] Note ON: {note} ({freq} Hz)")
        send_note(note, freq, 1)
        time.sleep(0.25)
        send_note(note, freq, 0)
        time.sleep(0.05)

if __name__ == "__main__":
    play_arpeggio()
