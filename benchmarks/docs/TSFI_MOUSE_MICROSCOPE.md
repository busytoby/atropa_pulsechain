# TSFi Mouse Microscope: The Shutter & Genetic Resonance

**Status:** THEORETICAL / PROTOTYPE
**Component:** \`plugins/vulkan/input_vty.c\`
**Integration:** TSFi Genetic Crossover (Wave512)

## 1. The Mouse as an Inverse Microscope

We reject the notion of the mouse as a 2D pointer. Instead, we define it as a **High-Speed Infrared Surface Scanner**. The sensor (e.g., PixArt PAW3395) operates as a monochromatic camera using grazing-angle IR illumination to capture the micro-topography of the tracking surface.

### Optical Architecture
*   **Source:** 850nm Infrared LED/Laser (VCSEL).
*   **Optics:** Dark-field imaging. Only surface irregularities (fiber edges, dust, texture) scatter light into the aperture. Smooth surfaces appear black; textured surfaces appear as constellations of high-contrast points.
*   **Resolution:** 30x30 to 40x40 pixel grid (typical).
*   **Shutter (Frame Rate):** Variable, up to **30,000 FPS**.

## 2. The Shutter: Temporal Super-Resolution

The tracked coordinate $(x, y)$ is merely the *integral* of thousands of micro-measurements. The true data lies in the **Shutter Speed** and **Correlation Quality**.

### 2.1 The 30,000 FPS Reality
At 30k FPS, the sensor captures a frame every ~33 microseconds.
*   **Human Scale:** A hand moving at 1 m/s travels only 33 micrometers between frames.
*   **Micro-Scale:** The sensor sees the surface tracking beneath it like a low-orbit satellite scanning terrain.

### 2.2 Analysis Capabilities
By analyzing the *timing jitter* and *event density* of the reported USB packets (typically 1kHz - 8kHz), we can infer the internal state of the sensor's DSP:

*   **Surface Complexity (SQUAL Proxy):**
    *   *High Complexity:* Sharp, high-contrast features allow the DSP to lock correlation instantly. USB reports are rhythmic and precise.
    *   *Low Complexity:* Glass or glossy plastic forces the DSP to gain-up (longer exposure), introducing latency and jitter.
*   **Micro-Tremor (The Biological Signal):**
    *   Even when "still," the human hand vibrates at 8-12Hz (physiological tremor). The sensor sees this as sub-pixel oscillation.
    *   This vibration is a **unique biometric signature** and a source of true physical entropy.

## 3. Feeding the Vulkan Genetic Crossover

The TSFi Genetic Engine (\`src/tsfi_genetic.c\`) usually relies on PRNGs for mutation. The Mouse Microscope replaces synthetic randomness with **Physical Resonance**.

### 3.1 Velocity Flux $\rightarrow$ Mutation Rate
*   **Logic:** $Rate_{mutation} = f(|\vec{v}|, |\vec{a}|)$
*   **Mechanism:**
    *   **Stillness (Micro-Tremor):** Low mutation, high stability. The system "crystallizes" around the current best fit.
    *   **High Flux (Rapid Movement):** High energy injection. The genetic algorithms enter a "chaotic" phase, breaking local minima and exploring vast solution spaces.
    *   **Jerk (Sudden Stops):** Triggers "Catastrophic Crossover" events, forcing immediate recombination of parent genomes.

### 3.2 Surface Texture $\rightarrow$ Entropy Pool
The microscopic irregularities of the mousepad serve as the "Seed" for procedural generation.
*   **Mapping:** The specific tracking noise (error variance) is fed directly into the AVX-512 register banks (\`ZMM\` registers) as entropy.
*   **Effect:** A rougher surface (Cordura) might generate "gritty," high-frequency genetic variants. A smooth surface (Speed pad) generates smooth, flowing topology. Tracking becomes a form of **Material Sampling**.

### 3.3 The Shutter as a Gatekeeper
We treat the sensor's Frame Rate as the **Simulation Clock**.
*   Instead of \`vsync\` (60Hz), the simulation steps are driven by tracking events (1000Hz+).
*   **No Movement = No Time:** If the "microscope" sees no change, the simulation freezes in a quantum superposition.
*   **Observation Collapses the Wavefunction:** Only by mechanically interacting with the surface (moving the mouse) does the system compute the next genetic generation.

## 4. Implementation Strategy

### A. High-Precision Polling (\`input_vty.c\`)
We must bypass standard \`read()\` blocking. We need \`ioctl(EVIOCG... )\` to query high-resolution timestamps.
\`\`\`c
struct input_event ev;
// Use ev.time (struct timeval) for microsecond precision
uint64_t delta_us = (ev.time.tv_sec * 1000000 + ev.time.tv_usec) - last_us;
\`\`\`

### B. Signal Derivative Calculation
Compute the **Jerk** (3rd derivative of position) to detect "texture hits."
\`\`\`c
float velocity = dx / dt;
float accel = (velocity - last_velocity) / dt;
float jerk = (accel - last_accel) / dt;
// Jerk > Threshold => Mutation Trigger
\`\`\`

### C. ZMM Injection
Directly map the input struct to a Wave512 register.
\`\`\`c
__m512i entropy = _mm512_set_epi64(ev.time.tv_sec, ev.time.tv_usec, ev.code, ev.value, ...);
// XOR into the global genetic state
\`\`\`

## 5. Surface Fingerprinting & Absolute Positioning

Standard optical mice are relative positioning devices; they accumulate drift over time. **Surface Fingerprinting** proposes that by memorizing the microscopic texture features of the tracking surface, the mouse can achieve **Absolute Localization** on a known pad.

### 5.1 The Texture Map Concept
Just as a star tracker identifies a spacecraft's orientation by recognizing constellations, the VTY can identify the mouse's absolute position $(X_{abs}, Y_{abs})$ by recognizing "micro-constellations" of surface imperfections (fiber knots, scratches, dust).

### 5.2 Implementation via Indirect Metrics
Since we cannot access the raw image buffer (without custom firmware), we use **Indirect Fingerprinting**:
*   **Sequence Hashing:** A specific 5cm scan across a "rough" patch of the mousepad produces a unique time-series signature of Velocity Jitter and Lift-Off variance.
    *   $Hash(Path) = \int_{t_0}^{t_1} (Jitter(t) \oplus SQUAL(t)) dt$
*   **Landmark Recognition:** A deep scratch or distinct weave anomaly acts as a "Lighthouse." When the sensor traverses it, the tracking engine reports a characteristic "hiccup" or spike in the \`REL_WHEEL_HI_RES\` or unmasked error registers.

## 6. The Hierarchy of Perception

The metrics derived from the 30,000 FPS raw shutter data form a tripartite sensory system for the TSFi engine, projecting the physical ground into simulation variables.

### 6.1 Surface Quality (SQUAL): The Sense of "Clarity"
SQUAL represents the number of valid features identifying a single IR frame. It is the biological analog of **Visual Acuity**.
*   **Simulation Role:** Environmental Coherence. High SQUAL indicates a stable physical ground; low SQUAL triggers **Evolutionary Drift**, forcing genomes to rapidly adapt to a perceived shifting reality.

### 6.2 Surface Hash: The Sense of "Memory"
Temporal-spatial fingerprints enable **Object Recognition**.
*   **Simulation Role:** Identity and Absolute Localization. The system recognizes specific physical desk coordinates via hash-match, enabling **Geographic Genomic Storage**. Physical spots on the desk "hold" genetic lineages.

### 6.3 Jerk Metric: The Sense of "Impact"
Jerk ($d^3x/dt^3$) detects transitions and collisions. It is the biological analog of **Nociception (Touch)**.
*   **Simulation Role:** Tactile Resonance and Universal Interrupts. Spikes in Jerk (hitting a texture snag or flicking the mouse) trigger **Catastrophic Crossover**, jolting genomes into new structural configurations via recombination.

### Summary of Perception Layers

| Metric | Derivation | Biological Analog | Simulation Role |
| :--- | :--- | :--- | :--- |
| **SQUAL** | Snapshot Contrast | Vision (Clarity) | Integrity / Mutation Rate |
| **Hash** | Spatial Sequence | Memory (Identity) | Absolute Positioning / Auth |
| **Jerk** | 3rd Deriv Motion | Touch (Impact) | Interrupts / Crossover Logic |
