# Audio Synthesis and Video Generation Reference

This document covers the media synthesis subsystems, including audio modeling algorithms and video performance renderers.

---

## 1. Audio Synthesis & DSP Subsystems

The repository contains emulators and oscillators for historical and customized synthesis architectures:

### A. Syrinx Formant Speech Synthesizer
* **Scripts**: [scripts/generate_syrinx_wav.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/generate_syrinx_wav.py), [scripts/bionika_syrinx_speech.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/bionika_syrinx_speech.py), [scripts/live_syrinx_performance.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/live_syrinx_performance.py)
* **Description**: Formant-based phonetic voice model reproducing human vocal paths and vowel frequencies using resonant filter sweep bands.
* **Output**: Produces speech waves (e.g. `bionika_syrinx_speech.wav`).

### B. TeddyBear 303 Acid Synthesizer
* **Scripts**: [scripts/generate_tb303_wav.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/generate_tb303_wav.js), [scripts/generate_acid_beat_wav.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/generate_acid_beat_wav.js)
* **Description**: Emulates the classic Roland TB-303 transistor ladder filter decay and slide parameters.
* **Output**: Produces acid baseline loops (e.g. `teddy_bear303_acid_synthesis.wav`).

### C. 808 & Booty Bass Generators
* **Scripts**: [scripts/generate_808_wav.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/generate_808_wav.js), [scripts/generate_booty_bass_wav.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/generate_booty_bass_wav.js)
* **Description**: Synthesizes deep decay kick oscillators and sub-bass transients.

---

## 2. Dynamic Video Generation

Visual performance tracks are rendered procedurally by combining audio signals with 3D coordinate trajectories:

### A. Ambient Manifold Visualizer
* **Script**: [scripts/generate_photorealistic_manifold_video.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/generate_photorealistic_manifold_video.py)
* **Description**: Maps multi-dimensional manifold geometries over time, modulating depth layers and line-art segments based on the audio frequency spectrum.
* **Outputs**: `ambient_manifold_composed_final.mp4` / `ambient_manifold_resonance.mp4`.

### B. Bouncy Wheel Performance
* **Script**: [scripts/generate_bouncy_wheel_performance.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/generate_bouncy_wheel_performance.py)
* **Description**: Uses a Verlet integrator model to compute soft-body elastic collisions (FET discharge mechanics) on a rolling 2D circular boundary.
* **Output**: `bouncy_wheel_performance.mp4`.

### C. Yi-Lau Evolution Projections
* **Script**: [scripts/generate_yi_lau_evolution.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/generate_yi_lau_evolution.py)
* **Description**: Translates historic **Auncient** math lookup indexes to coordinate plots of hypotrochoids.
* **Output**: `yi_lau_evolution_demo.mp4`.
