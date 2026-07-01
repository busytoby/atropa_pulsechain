import numpy as np

MotzkinPrime = 953467954114363

BEARS = [
    {"id": 0, "abbr": "JDG", "name": "Judge Bear (Alpha)", "exp": 440},
    {"id": 1, "abbr": "WTA", "name": "Witness A (Beta)", "exp": 12000},
    {"id": 2, "abbr": "WTB", "name": "Witness B (Gamma)", "exp": 8877},
    {"id": 3, "abbr": "JR1", "name": "Juror 1 (Delta)", "exp": 4433},
    {"id": 4, "abbr": "JR2", "name": "Juror 2 (Epsilon)", "exp": 5522}
]

EMOTIONS = {
    "JOY": 1009,
    "SORROW": 1543,
    "ANGER": 2237,
    "FEAR": 3109,
    "MELANCHOLY": 4423,
    "EUPHORIA": 5519,
    "SUSPENSE": 6701,
    "TRANQUILITY": 7883,
    "CONFUSION": 8923
}

# The 3 unique chords played by each bear (variations on Melancholy voltage)
CHORDS = [
    {"id": 1, "offset": -200, "desc": "Low Melancholy sweep"},
    {"id": 2, "offset": 0,    "desc": "Baseline Melancholy"},
    {"id": 3, "offset": 350,  "desc": "High Melancholy drone"}
]

def mod_exp(base, exp, mod):
    res = 1
    base = base % mod
    while exp > 0:
        if exp % 2 == 1:
            res = (res * base) % mod
        base = (base * base) % mod
        exp //= 2
    return res

def derive_hypotrochoid(voltage, exp):
    resolved = mod_exp(voltage, exp, MotzkinPrime)
    R = (resolved % 50) + 15.0
    r = (resolved % 25) + 8.0
    d = (resolved % 15) + 5.0
    return R, r, d

# Generate the self-observation databases for all 5 bears
# Each bear plays the 9 emotions on its own hardware and logs the resulting ratio R/r
BEAR_DATABASES = []
for b in BEARS:
    db = {}
    for emo, voltage in EMOTIONS.items():
        R, r, d = derive_hypotrochoid(voltage, b["exp"])
        ratio = R / r if r > 0.0 else 1.0
        db[emo] = ratio
    BEAR_DATABASES.append(db)

# Self-projection classifier: observer compares speaker's ratio to its own database
def classify_by_projection(observer_id, speaker_R, speaker_r):
    speaker_ratio = speaker_R / speaker_r if speaker_r > 0.0 else 1.0
    db = BEAR_DATABASES[observer_id]
    
    best_emo = "CONFUSION"
    min_diff = float('inf')
    
    # Find the emotion in observer's database that maps to the closest ratio
    for emo, target_ratio in db.items():
        diff = abs(target_ratio - speaker_ratio)
        if diff < min_diff:
            min_diff = diff
            best_emo = emo
            
    return best_emo

def main():
    print("[GRADING-PANEL] Running immediate grading panel across all five bears...\n")
    
    for section_idx, speaker in enumerate(BEARS):
        print(f"=====================================================================")
        print(f"SECTION {section_idx + 1}: CHORDS PRODUCED BY {speaker['name'].upper()}")
        print(f"=====================================================================")
        
        print(f"{'Chord Played':<25} | {'JDG':<8} | {'WTA':<8} | {'WTB':<8} | {'JR1':<8} | {'JR2':<8}")
        print("-" * 75)
        
        for c in CHORDS:
            voltage = EMOTIONS["MELANCHOLY"] + c["offset"]
            
            # Speaker resolves the physical shape
            R, r, d = derive_hypotrochoid(voltage, speaker["exp"])
            
            # All 5 bears classify the speaker's spirograph by comparing to their own database
            grades = []
            for observer in BEARS:
                emo = classify_by_projection(observer["id"], R, r)
                grades.append(emo)
                
            print(f"{c['desc']:<25} | {grades[0]:<8} | {grades[1]:<8} | {grades[2]:<8} | {grades[3]:<8} | {grades[4]:<8}")
        print()

if __name__ == "__main__":
    main()
