# TTS & Vocoder Dashboard Improvements Roadmap

To take the dashboard from its current functional state to a state-of-the-art, premium retro-computing control center, we recommend implementing the following five modules.

---

## 1. 🎚️ Interactive Pitch Contour & Intonation Editor (Prosody)
Currently, phonemes are spoken at a fixed singing base pitch, which sounds robotic. To make speech expressive or natural, we can build a graphical pitch contour editor.

*   **Interactive SVG/Canvas Envelope Grid**: A draggable curve editor below the Phoneme Palette. Users can draw a line to set the pitch curve (e.g., rising at the end for a question, falling at the end of a sentence).
*   **Contour Exporter**: The drawn pitch contour will map to the 24-byte pitch registers passed to the on-chain Solidity/Yul vocoder, matching the smart contract's autotune capability.

## 2. 🎛️ Preset Speaker Profiles
Instead of manually tweaking knobs, users should be able to select preconfigured voice profiles:

*   **"Robby the Robot"**: Enforces GI SP0256-AL2 12-stage LPC mode, $10\text{ kHz}$ output, high jitter, and robotic impulse excitation.
*   **"Syd 64"**: Enforces C64 4-bit Digi mode with $6\text{ kHz}$ register-write carrier whine and retro chip noise floor.
*   **"Stephen"**: Enforces TMS5220 Speak & Spell LPC mode with the authentic 52-sample TI ROM Chirp excitation.
*   **"Breathy Human"**: Enforces Humanized LPC mode with Liljencrants-Fant glottal flow waveforms, vibrato, and phase-locked aspiration noise.

## 3. 📊 Real-Time Formant & Spectrum Analyzer
Replace the current placeholder oscilloscope with a live Web Audio analyser:

*   **Dual View Visualizer**: 
    1.  **Oscilloscope**: Shows the raw synthesized time-domain waveform.
    2.  **FFT Spectrum**: Shows frequency bars in real-time, highlighting the locations of formants ($F_1, F_2, F_3$) as the voice peaks.
*   **Cyberpunk Neon Styling**: Responsive canvas styled with custom glowing gradients matching the active synthesizer chip's theme.

## 4. 🔗 Advanced Web3 Diagnostics Panel
Enhance the smart contract integration UI:

*   **EVM State Polling**: Real-time ticker showing Anvil testnet connection status, contract balances, gas limits, and transaction history.
*   **Offline Mode Bypass Indicator**: A prominent status indicator showing whether you are testing locally or broadcasting speech frame projection transactions directly to the blockchain.

## 5. 🎨 Premium Glassmorphism & Cyberpunk Aesthetics
Overhaul the UI layout using modern front-end CSS standards:

*   **Theme Coordinates**: Add glowing borders, box-shadow pulses, and glassmorphic card backdrops (`backdrop-filter: blur(12px)`).
*   **Smooth Micro-Animations**: Vowels animate the visual mouth height smoothly using CSS transitions, and active phoneme chips pulse with a glowing border as they play.
