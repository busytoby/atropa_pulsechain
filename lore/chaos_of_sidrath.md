# Chaos Of Sidrath
**Genre:** Auncient Liquid Neurofunk / Drum & Bass  
**Tempo:** 174 BPM  
**Scale Key:** C Minor (C, D, Eb, F, G, Ab, Bb)  
**Hardware Target:** MOS 6581 SID (musicMaker.yul emulation) / 3-Voice Array  

---

## 1. Sound Design & Patch Configuration

### Voice Channel 1: Sidrath Sub-Attractor (Bassline)
* **Waveform:** Triangle + Variable Pulse Width (PWM) 
* **Filters:** 4-Pole Lowpass Diode-Ladder cascade with 60% resonance.
* **Envelope (ADSR):** `Attack: 2ms`, `Decay: 180ms`, `Sustain: 80%`, `Release: 120ms`
* **Modulation:** LFO modulation mapped to PWM width (cycle rate: 5.8Hz) to create the growling negative-resistance wobble.

### Voice Channel 2: Trigonometric Chaos (Lead & Arpeggiator)
* **Waveform:** Sawtooth + Ring Modulation (Voice 1 $\times$ Voice 2 carrier)
* **Filters:** Highpass Bandpass Parallel split.
* **Envelope (ADSR):** `Attack: 0ms`, `Decay: 75ms`, `Sustain: 15%`, `Release: 50ms`
* **Modulation:** Postfix RPN arpeggiator sweeping through Sidrath chord scales (Minor 9th, Flat 5th inversions).

### Voice Channel 3: White Noise Gate (Drums/Percussion)
* **Waveform:** Filtered White Noise (LCG Noise generator) + High-Frequency Sine
* **Envelope (ADSR):**
  * *Kick Mode:* `Decay: 90ms`, lowpass sweep from 150Hz down to 45Hz.
  * *Snare Mode:* `Decay: 65ms`, bandpass noise overlay centered at 1.2kHz.

---

## 2. Complete Song Sequence Array

| Pattern Index | Section Name | Measures | Description |
| :--- | :--- | :--- | :--- |
| `00` | Intro (The Awakening) | 1–8 | Sub-bass sweeps, sparse arpeggio hints, ticking hats. |
| `01` | Build-Up (Attractor Warp) | 9–16 | Fast drum rolling, arpeggiator acceleration, rising filters. |
| `02` | Drop A (Chaos Inversion) | 17–24 | Heavy bass growls, fully opened highpass leads, main drum loop. |
| `03` | Drop B (Sidrath Decapitation) | 25–32 | Modulated pitch glides on bass, complex syncopated lead patterns. |
| `04` | Breakdown (RPN Resolution) | 33–40 | Rhythmic snare rolls, clean triangle chords, no sub growls. |
| `05` | Outro (Fading Horizon) | 41–48 | Return of simple ticking, resolving tones, sine decay. |

---

## 3. Detailed Step Scores (16-Step Grids)

### Pattern 00: Intro
```text
Step | Voice 1 (Bass)   | Voice 2 (Arp) | Voice 3 (Drums/Perc) | FX / Sweeps
-----------------------------------------------------------------------------
01   | C-2 (Sub Low)    | -             | -                    | Filter cutoff @ 120Hz
02   | -                | -             | Closed Hat           | -
03   | -                | -             | -                    | -
04   | -                | -             | Closed Hat           | -
05   | -                | -             | -                    | -
06   | -                | -             | Closed Hat           | -
07   | -                | -             | -                    | -
08   | -                | Eb-4          | Open Hat             | -
09   | C-2 (Sub Low)    | -             | -                    | Filter cutoff @ 135Hz
10   | -                | -             | Closed Hat           | -
11   | -                | -             | -                    | -
12   | -                | -             | Closed Hat           | -
13   | -                | -             | -                    | -
14   | -                | -             | Closed Hat           | -
15   | -                | -             | -                    | -
16   | -                | G-4           | Open Hat             | -
```

### Pattern 01: Build-Up
```text
Step | Voice 1 (Bass)   | Voice 2 (Arp) | Voice 3 (Drums/Perc) | FX / Sweeps
-----------------------------------------------------------------------------
01   | C-2 (Sub Low)    | C-4           | Kick                 | Arp tempo acceleration
02   | -                | Eb-4          | Closed Hat           | -
03   | -                | G-4           | Snare (Soft)         | -
04   | -                | Bb-4          | Closed Hat           | -
05   | C-2 (Glide Up)   | D-5           | Kick                 | Filter cutoff rise
06   | -                | Bb-4          | Closed Hat           | -
07   | -                | G-4           | Snare (Soft)         | -
08   | -                | Eb-4          | Snare (Soft)         | -
09   | Eb-2             | C-4           | Kick                 | -
10   | -                | D-4           | Snare (Roll)         | -
11   | -                | G-4           | Snare (Roll)         | -
12   | -                | Bb-4          | Snare (Roll)         | -
13   | F-2              | D-5           | Snare (Accent)       | Gate fully open
14   | -                | G-4           | Snare (Accent)       | -
15   | -                | Eb-4          | Snare (Accent)       | -
16   | -                | D-4           | Snare (Accent)       | -
```

### Pattern 02: Drop A (Chaos Inversion)
```text
Step | Voice 1 (Bass)   | Voice 2 (Arp) | Voice 3 (Drums/Perc) | FX / Sweeps
-----------------------------------------------------------------------------
01   | C-2 (Heavy Growl)| C-5           | Kick                 | Filter sweeps @ 480Hz
02   | -                | G-4           | Closed Hat           | PWM Mod depth 90%
03   | -                | Eb-4          | -                    | -
04   | -                | Bb-4          | Closed Hat           | -
05   | F-2 (Wobble Mid) | C-5           | Snare                | Triode overdrive on
06   | -                | G-4           | -                    | -
07   | -                | Bb-4          | Closed Hat           | -
08   | -                | Eb-4          | Open Hat             | -
09   | Eb-2 (Heavy Growl)| C-5          | Kick                 | -
10   | -                | G-4           | -                    | -
11   | -                | Eb-4          | Closed Hat           | -
12   | -                | Bb-4          | Kick (Syncopated)    | -
13   | Ab-2 (Sub Drop)  | C-5           | Snare                | -
14   | -                | G-4           | -                    | -
15   | -                | Bb-4          | Closed Hat           | -
16   | G-2 (Attractor)  | D-4           | Open Hat             | -
```

### Pattern 03: Drop B (Sidrath Decapitation)
```text
Step | Voice 1 (Bass)   | Voice 2 (Arp) | Voice 3 (Drums/Perc) | FX / Sweeps
-----------------------------------------------------------------------------
01   | C-2 (Heavy Growl)| Bb-4          | Kick                 | Pitch bend on Voice 1
02   | -                | G-4           | Closed Hat           | -
03   | -                | Eb-4          | -                    | -
04   | -                | C-4           | Kick                 | -
05   | Bb-1 (Low Rumble)| D-5           | Snare                | Highpass cutoff sweep
06   | -                | G-4           | -                    | -
07   | -                | F-4           | Closed Hat           | -
08   | -                | Eb-4          | Open Hat             | -
09   | Ab-2 (Sub Drop)  | Bb-4          | Kick                 | -
10   | -                | G-4           | -                    | -
11   | -                | Eb-4          | Closed Hat           | -
12   | -                | C-4           | Kick (Syncopated)    | -
13   | G-2 (Glide Out)  | D-5           | Snare                | -
14   | -                | G-4           | -                    | -
15   | -                | F-4           | Closed Hat           | -
16   | F-2              | D-4           | Open Hat             | -
```

### Pattern 04: Breakdown
```text
Step | Voice 1 (Bass)   | Voice 2 (Arp) | Voice 3 (Drums/Perc) | FX / Sweeps
-----------------------------------------------------------------------------
01   | C-2 (Clean Tri)  | C-4           | -                    | Low resonance (soft)
02   | -                | Eb-4          | Closed Hat           | -
03   | -                | G-4           | -                    | -
04   | -                | Bb-4          | Closed Hat           | -
05   | -                | D-5           | Snare (Rimshot)      | -
06   | -                | Bb-4          | -                    | -
07   | -                | G-4           | Closed Hat           | -
08   | -                | Eb-4          | Open Hat             | -
09   | Eb-2 (Clean Tri) | C-4           | -                    | -
10   | -                | D-4           | Closed Hat           | -
11   | -                | G-4           | -                    | -
12   | -                | Bb-4          | Closed Hat           | -
13   | -                | D-5           | Snare (Rimshot)      | -
14   | -                | G-4           | -                    | -
15   | -                | Eb-4          | Closed Hat           | -
16   | -                | D-4           | Snare (Roll)         | -
```

### Pattern 05: Outro
```text
Step | Voice 1 (Bass)   | Voice 2 (Arp) | Voice 3 (Drums/Perc) | FX / Sweeps
-----------------------------------------------------------------------------
01   | C-2 (Sub Fade)   | C-4           | -                    | Decaying amplitude
02   | -                | Eb-4          | Closed Hat           | -
03   | -                | -             | -                    | -
04   | -                | -             | Closed Hat           | -
05   | -                | -             | -                    | -
06   | -                | -             | -                    | -
07   | -                | -             | Closed Hat           | -
08   | -                | -             | Open Hat             | -
09   | -                | -             | -                    | -
10   | -                | -             | Closed Hat           | -
11   | -                | -             | -                    | -
12   | -                | -             | Closed Hat           | -
13   | -                | -             | -                    | -
14   | -                | -             | -                    | -
15   | -                | -             | -                    | -
16   | -                | -             | -                    | -
```
