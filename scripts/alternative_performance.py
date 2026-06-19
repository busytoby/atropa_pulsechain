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
        pass

def play_slow_pentatonic():
    print("[SLOW PERFORMANCE] Starting a 15-second slow pentatonic progression...")
    
    melody = [
        ("A3", 220.00),
        ("C4", 261.63),
        ("D4", 293.66),
        ("E4", 329.63),
        ("G4", 392.00),
        ("A4", 440.00),
        ("C5", 523.25),
        ("D5", 587.33),
        ("E5", 659.25),
        ("G5", 783.99),
    ]
    
    for note, freq in melody:
        print(f"[SLOW PERFORMANCE] Playing Note: {note} ({freq:.2f} Hz)")
        send_note(note, freq, 1)
        time.sleep(1.2) # Hold note for 1.2 seconds
        send_note(note, freq, 0)
        time.sleep(0.3) # Space between notes

if __name__ == "__main__":
    play_slow_pentatonic()
