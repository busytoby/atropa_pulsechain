# hal_demo_video_artifact.md

This artifact documents the generated 45-second high-fidelity MP4 video demonstration of the Auncient EDSAC Hardware Abstraction Layer (HAL) operating on the H-Bridge and Register Allocation Units (RAU).

### Asset Details:
* **Output Video File**: [hal_hbridge_demo.mp4](file:///home/mariarahel/.gemini/antigravity-cli/brain/dd413f30-1457-4127-8d11-415844cbbc2d/hal_hbridge_demo.mp4)
* **Format**: 1280x720 Progressive H.264 Video @ 30 FPS, AAC audio.
* **Duration**: 45 seconds.

### Technical Elements Visualized:
1. **Coaxial RAU Status Monitor**: Displays real-time contents of `RAU_DITHER_IN`, `RAU_DITHER_OUT`, and `RAU_DITHER_ERR` registers.
2. **Scratchpad Oscilloscope**: Renders sector values (`SEC_00` through `SEC_05`) representing raw delay-line acoustic pulse data.
3. **EDSAC Parsing logs**: Displays the compiled trace of incoming symbolic bootstrap load instructions.
4. **Geometric Quaternion Wireframe**: Shows a rotating wireframe structure whose line density and size pulse to the beat of the soundtrack.

### Audio Track Synthesis:
* **Kick Drum**: Modelled via rapid exponential pitch sweep ($150\text{ Hz} \to 45\text{ Hz}$) and amplitude decay envelope ($e^{-15t}$).
* **Snare Drum**: Modelled via high-frequency white noise bursts with sharp decay envelopes ($e^{-22t}$).
* **Tempo**: 120 BPM synchronized to the visual elements.
