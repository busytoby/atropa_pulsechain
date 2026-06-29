# Progressive 12-Minute Choreography & DSP Plan

This document outlines the musical arrangement and rendering plan for the extended vertical loop (720s) utilizing the Auncient Dysnomia synthesizer and vision suite.

## 1. Synthesizer & Sequencer Upgrades
To support a 12-minute progressive track, the DSP engine in `make_chaos_short_loop.py` will be upgraded with the following modules:

*   **Atmospheric Chord Pad (Channel 4):** A fourth voice layer using four detuned sine waves to play harmonic minor triad pads with slow attack/decay curves.
*   **Feedback Echo / Delay:** A ring-buffer based delay line on the lead arpeggiator channel with a 3-beat delay time and 40% feedback decay.
*   **Envelope Filter Sweep:** A dynamic low-pass sweep envelope applied to the Bass saw/triangle mix to simulate analog resonance sweeps.

---

## 2. Arrangement Timeline (720 Seconds / 12 Minutes)
At 174 BPM, 12 minutes is exactly 2088 beats. We divide this into eight 90-second blocks (261 beats each) forming a seamless loop:

### Phase 1: 0s – 90s | Ambient Awakening (Intro)
*   **Audio:** Only the new Atmospheric Chord Pad (Channel 4) playing a dark minor progression, with sparse hats. Bass and Lead remain silent.
*   **Visuals:** Plasma background moves slowly. Starfield travels at minimum speed. Grid floor is static. The 4D Tesseract rotates on a single plane.

### Phase 2: 90s – 180s | Arpeggio Rise (Verse 1)
*   **Audio:** Arpeggiator (Channel 2) enters with the fast 174 BPM lead. Sub-bass enters with low drone patterns. Simple kick drum enters on beats 1 and 3.
*   **Visuals:** Starfield speeds up. Torus starts orbiting the tesseract. Grid floor begins scrolling slowly.

### Phase 3: 180s – 270s | The Attractor Drop (Halftime Drop 1)
*   **Audio:** Halftime drum loop triggers. Bass octave drops with heavy LFO sweeps. Lead slows to legato theme with echo trails active.
*   **Visuals:** Lorenz Attractor trail expands and glows bright phosphor green. Starfield pulses on beats. Torus contracts/expands.

### Phase 4: 270s – 360s | Attractor Chaos (Climax 1)
*   **Audio:** Full neurofunk breakbeat triggers. Lead switches to detuned dual-voice unison arpeggios. Wobble bass runs at 12Hz.
*   **Visuals:** Tesseract scales up and morphs rapidly on three rotation planes. Grid perspective shifts side-to-side.

### Phase 5: 360s – 450s | Ambient Break (The Valley)
*   **Audio:** High-energy channels cut. Lead arpeggio slowly decays. Atmospheric pads return. Low sub drone.
*   **Visuals:** Visual elements scale down. Attractor fades. Screen brightness dims to a subtle state.

### Phase 6: 450s – 540s | The Double Build-up
*   **Audio:** Rapid double-time kick drum roll. Pad sweep filter opens. Lead arpeggio pitch rises continuously.
*   **Visuals:** Grid floor scroll speed doubles. Starfield particles swell and glow. Tesseract outline thickens.

### Phase 7: 540s – 630s | Hyper Climax (Drop 2)
*   **Audio:** Full energy breakbeat. Unison detuned lead arpeggios. Heavy resonant bass sweeps.
*   **Visuals:** Torus, Attractor, and Tesseract rotate in counter-phases. Spectrum bars reach peak heights.

### Phase 8: 630s – 720s | Descent & Outro (Loop Transition)
*   **Audio:** Breakbeat fades to simple hats. Bass returns to intro drone. Pad progression resolves back to the key chord. Lead arpeggio pitch slides down to original octave.
*   **Visuals:** Starfield slows to intro speed. Torus orbits slow down. Grid floor returns to a static state.

---

## 3. Seamless Loop Mathematical Conditions
To ensure the transition from 719.9s to 0.0s is invisible:
*   **Rotations:** All angles $\theta$ and $\phi$ use `(time / DURATION) * 2.0 * math.pi * N` to ensure start and end states are identical.
*   **Grid Scroll:** Scrolling speed `grid_speed` is set to an integer multiple of the floor grid lines over the 720-second duration.
*   **Audio Envelopes:** Zero-crossing crossfades are applied to the start and end of all synthesizer output buffers.
