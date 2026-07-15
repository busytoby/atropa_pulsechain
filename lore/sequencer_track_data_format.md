# TSFi2 Sequencer Track Data Format Specification

This document defines the structured JSON data format for representing polyphonic synthesizer and drum machine sequences within the TSFi2 Synthesis Studio. These Bionika sequence files are saved with the **`.bio`** file extension.

---

## 1. Schema Definition

Sequences are represented as JSON objects. To support full song arrangements without massive duplicate data arrays, the format uses a modular **Pattern Playlist** paradigm:
*   **`patterns`**: An object containing reusable 16-step or 32-step musical blocks (e.g. `intro`, `verse`, `chorus`). Each pattern defines its own instrument and drum channels.
*   **`arrangement`**: An ordered list of pattern names determining the performance timeline.

```json
{
  "name": "Track Title String",
  "tempo": 120,
  "swing": 0.32,
  "arrangement": ["intro", "verse", "chorus", "verse", "chorus"],
  "patterns": {
    "intro": {
      "bass": {
        "sequence": ["C2", "C2", "C3", "C2", "Eb2", "G2", "Bb2", "C3", "C2", "C3", "C2", "Eb2", "G2", "Bb2", "C3", "C2"],
        "accents": [0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0],
        "slides": [0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 1]
      },
  },
  "drums": {
    "kick": [1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0],
    "snare": [0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0],
    "closed_hh": [0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1],
    "open_hh": [0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0],
    "cowbell": [0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0],
    "clap": [0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 2, 1]
  }
}
```

---

## 2. Parameter Specifications

### A. Global Control
*   **`name`** (string): Descriptive title for the track performance.
*   **`tempo`** (number): Speed of the performance in Beats Per Minute (BPM).
*   **`swing`** (number): Rhythmic shuffle ratio applied to alternate 16th notes, ranging from `0.0` (straight) to `0.5` (maximum swing/shuffle).
*   **`steps`** (integer): The looping step length of the primary sequences (typically 16 or 32 steps).

### B. Bass Synthesizer Channel (`bass`)
*   **`sequence`** (array of strings): Note names (e.g. `"C2"`, `"Eb3"`) determining the oscillator frequency.
*   **`accents`** (array of integers): Velocity modifiers where `1` indicates an accented note (higher filter cutoff envelope depth) and `0` indicates normal velocity.
*   **`slides`** (array of integers): Pitch glides where `1` forces the note pitch to slide smoothly to the next note's frequency, and `0` triggers a standard decay envelope.

### C. Drum Trigger Channels (`drums`)
Rhythmic trigger sequences represented as integer arrays matching the track step count:
*   **`kick`** / **`snare`** / **`closed_hh`** / **`open_hh`** / **`cowbell`**: Value of `1` triggers the sample/synthesis envelope, and `0` represents a rest.
*   **`clap`**: Dynamic clap trigger where `1` triggers a single handclap, `2` triggers a double-clap (flam), and `0` represents a rest.

### D. Sub-Bass Growl Channel (`sub_growl`)
Used to fill out the bottom end with subtle, low-volume harmonic textures layered beneath kick drums:
*   **`sequence`** (array of strings): Low-frequency note triggers (e.g. `"C1"`, `"Bb0"`) driving sub-octave oscillators.
*   **`modulation_rate`** (array of numbers): LFO frequencies (typically centered around $5.8\text{Hz}$) that sweep PWM widths or Zener diode noise index registers to create the negative-resistance "growl" wobble.
*   **`gain`** (array of numbers): Low amplitude scaling coefficients (ranging from `0.0` to `0.3`) ensuring the growl remains a subtle background texture without overpowering the main transient of the kick.

### E. Arpeggiator Filter Control (`arpeggiator_filter`)
Classified as a control modulation source (rather than an acoustic generator). It outputs control voltage (CV) steps that modulate the cutoff frequency parameter of the primary voice filters:
*   **`sequence`** (array of strings): Target note pitch values (e.g. `"C3"`, `"Eb3"`, `"G3"`) determining the stepping cutoff frequencies of the vocal/lead formant filters.
*   **`gate`** (array of integers): Trigger bits where `1` routes the control voltage step to open the filter envelope, and `0` leaves the filter closed/decaying.
