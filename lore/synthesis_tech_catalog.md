# TSFi/2 Audio-Visual Synthesis & Rendering Technology Catalog

This document catalogs our complete synthesizer and rendering technologies and defines how they interlock within the 44-minute Hardstyle choreography.

---

## 1. Synthesizer Technologies (Auncient DSP)

### A. Coupled Bilateral Syrinx Simulator
* **Physics**: 3-mass Verlet oscillator mapping epibar/hypobar stiffness, mucosal soft shell boundaries ($d_{soft}$), and rigid cartilage cores ($d_{hard}$).
* **Behavior**: Generates avian whistles, hoarse croaks, and bilateral vocal tract interaction.
* **Choreography Interlock**: Glottal flow ($A_g = \max(x_1, 0) \cdot l_g$) dynamically scales the primary scale dimensions of the Voxel Pedestal.

### B. OC71 Germanium Common-Emitter Stage
* **Physics**: 17-point diode voltage conduction table ($V_{be}$ LUT) with charging base capacitors.
* **Behavior**: Symmetrical base shunting distortion generating warm odd and even harmonics.
* **Choreography Interlock**: Collector current output ($I_c$) drives the saturation intensity and neon glow width of the rendering grid.

### C. Sprott Chaotic Jerk Oscillator
* **Physics**: 3rd-order differential system ($\dddot{x} + 0.6 \ddot{w} + \dot{w} + w = \text{sgn}(w)$).
* **Behavior**: Non-periodic deterministic chaotic noise and turbulence.
* **Choreography Interlock**: Positional state variables ($j_x, j_y$) are mapped as offset vectors for particle rendering.

### D. TB-303 Acid Bass & 4-Pole Diode Ladder
* **Physics**: 24dB/oct cascaded filter emulation with exponential RC glide/slide logic.
* **Behavior**: Self-oscillating liquid resonance sweeps.
* **Choreography Interlock**: Cutoff frequency envelope sweeps drive the rotational velocity of the 4D Tessaract.

### E. Bridged-T TR-808 Kick & Snare Traps
* **Physics**: Impulse-excited twin-T bandpass oscillation + wrapped noise traps.
* **Behavior**: Deep sub-bass decay and loose noise snares with absolute scale wrapping.
* **Choreography Interlock**: Kick triggers trigger immediate full-frame contrast flashes and HUD glitches.

---

## 2. Rendering & Animation Technologies (VLM Presenter)

### A. 4D Hyper-Tessaract Projection
* **Math**: 6-axis rotational projection ($XY, XZ, XW, YZ, YW, ZW$) from 4D space into 3D.
* **Behavior**: Non-Euclidean geometric warping.
* **Interlock**: Rotated and deformed via the 3rd-order Jerk derivative of the synthesizer's active voice.

### B. Voxel Card Generation (LangChain & VLM)
* **Math**: Isometric voxel cube projection with hidden-face removal.
* **Behavior**: 3D sci-fi trading card frames with live stats.
* **Interlock**: Boosted dynamically by on-chain `hypobar` / `epibar` statistics.

### C. Sprite Particle Echolocation
* **Math**: 2nd-order Verlet kinematics with lift and drag.
* **Behavior**: Spawning sparks that fly towards the agents on trigger beats.
* **Interlock**: Spawn rates and speed bounds are locked to the snare trigger velocity.

---

## 3. Choreography Timeline Matrix (44 Minutes)

| Timeframe | Track / Section | Time Signature | Synth Focus | Visual Scene | Audio-Video Interlock Hook |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **00:00 - 08:00** | Song 1: *Auncient Ingress* | 4/4 Hardstyle | TR-808 Kick, Syrinx (Crow Profile) | **Scene 1**: The Glottal Chamber | Glottal area flow maps to the vertical voxel scale. |
| **08:00 - 15:00** | Song 1: (Transition) | 13/8 Complex | Syrinx Bilateral Whistles | **Scene 2**: 4D Tessaract Projection | Bilateral coupling factor ($K_c$) controls Tessaract $XW$-plane shear. |
| **15:00 - 23:00** | Song 2: *Hertzian Boundary* | 7/8 Micro-Phase | TB-303 Acid, OC71 Fuzz | **Scene 3**: Germanium Warpfold Grid | Transistor current ($I_c$) drives color saturation and grid distortion. |
| **23:00 - 30:00** | Song 2: (Transition) | 5/4 Odd Phase | Warpfolded Formant SVF | **Scene 1**: Glottal Chamber | Modulo wrap-around triggers visual scanline glitching. |
| **30:00 - 38:00** | Song 3: *Jerk Eruption* | 13/8 Complex | Chaotic Jerk Snare, Noise Traps | **Scene 4**: Chaotic Jerk Flurry | 3rd-order Jerk oscillator output shakes agent joint coordinates. |
| **38:00 - 44:00** | Song 3: (Finale) | 200 BPM 4/4 | All Synthesizers (Full Power) | **Scene 5**: The Sovereign Ledger | Live contract stats and Accrued Diyat Fees drive the HUD graphs. |
