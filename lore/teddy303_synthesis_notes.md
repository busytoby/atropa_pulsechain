# Teddy Bear 303 Acid Bass & 808 Drum Synthesis

We have synthesized three high-fidelity audio demonstrations to showcase the standalone and mixed capabilities of the **Teddy 303 Acid Bass** and **Teddy 808 Kick Drum** engines.

---

## 1. Synthesis Output Players

### Demonstration A: Standalone 303 Acid Bassline
*16-step arpeggiated bass sequence featuring slide glides and cutoff sweep decay:*

![Teddy Bear 303 Acid Bassline](/home/mariarahel/.gemini/antigravity-cli/brain/53112525-9898-4973-a6fc-c208cc0d018f/teddy303_acid_synthesis.wav)

### Demonstration B: Standalone 808 Kick Drum
*Four-on-the-floor TR-808 Kick Drum pattern (deep sub-bass decay with punchy transient click):*

![Teddy Bear 808 Kick](/home/mariarahel/.gemini/antigravity-cli/brain/53112525-9898-4973-a6fc-c208cc0d018f/teddy808_kick_synthesis.wav)

### Demonstration C: Combined 303 Bassline & 808 Kick Beat
*The 303 bassline playing alongside the 808 Kick pattern:*

![Teddy Bear 303 & 808 Beat](/home/mariarahel/.gemini/antigravity-cli/brain/53112525-9898-4973-a6fc-c208cc0d018f/teddy303_808_acid_beat.wav)

---

## 2. DSP Engine Architecture & Mathematical Model

### Step 1: Sawtooth Oscillator Phase
A standard sawtooth waveform is synthesized by accumulating phase based on the target frequency:
$$\text{osc}(t) = 2 \cdot (\phi(t) \bmod 1) - 1$$
* **Portamento Slide Glide**: Pitch transitions are smoothed by interpolating the active frequency $f$ towards the target sequencer pitch $F_{\text{target}}$ using an RC filter constant:
  $$f_{\text{new}} = f_{\text{old}} + (F_{\text{target}} - f_{\text{old}}) \cdot k_{\text{slide}}$$

### Step 2: 4-Stage Diode-Ladder Lowpass Filter
To simulate the signature 24dB/oct diode lowpass ladder filter, we run a 4-pole differential equation cascade with negative feedback resonance:
$$x(t) = \text{osc}(t) - r \cdot p_3(t - 1)$$
$$p_0(t) = p_0(t-1) + c \cdot (x(t) - p_0(t-1))$$
$$p_1(t) = p_1(t-1) + c \cdot (p_0(t) - p_1(t-1))$$
$$p_2(t) = p_2(t-1) + c \cdot (p_1(t) - p_2(t-1))$$
$$p_3(t) = p_3(t-1) + c \cdot (p_2(t) - p_3(t-1))$$
Where $c$ is the dynamic cutoff coefficient scaled by the envelope sweep, and $r$ is the resonance feedback.

### Step 3: TR-808 Kick Drum Active Resonator Circuit
The TR-808 kick drum is modeled by simulating an active bridged-T resonator circuit driven by an initial trigger impulse:
* **Pitch Envelope Sweep**: When a trigger occurs, the pitch sweeps rapidly from 150Hz down to a deep sub-bass frequency (48Hz) within 40ms:
  $$f_{\text{kick}}(t) = 48 + 102 \cdot e^{-t / 0.038}$$
* **Click Transient**: A 3.5ms white-noise click transient is mixed in at trigger start to give the drum punch.
* **Resonator Decay**: The sub-bass decay is mapped to a slow exponential amplitude decay envelope (420ms):
  $$E_{\text{kick}}(t) = e^{-t / 0.42}$$
* **Formula**:
  $$\text{kick}(t) = \tanh((\sin(2\pi \cdot f_{\text{kick}}(t) \cdot t) + \text{click}(t)) \cdot E_{\text{kick}}(t) \cdot 1.6)$$

### Step 4: Triode Valve Overdrive Saturation
Both kick and bass channels are shaped non-linearly using a hyperbolic tangent saturation curve to emulate analog valve saturation, clipping the peak resonance spikes and introducing rich odd-harmonic overtones:
$$\text{out}(t) = \tanh(\text{signal}(t))$$

---

## 3. How to Regenerate Locally
You can regenerate these WAV files at any time by running:
* **For 303 Standalone**: `node scripts/generate_tb303_wav.js`
* **For 808 Standalone**: `node scripts/generate_808_wav.js`
* **For Combined Beat**: `node scripts/generate_acid_beat_wav.js`
The files will export directly to their respective filenames in the root workspace.
