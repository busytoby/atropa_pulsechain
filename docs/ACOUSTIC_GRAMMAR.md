# The Acoustic Grammar of Transfluxor Words

This document outlines the architectural specifications for the **Acoustic Grammar of Transfluxor Words**. In the PL/EXUS system, execution is not driven by alphanumeric characters or byte opcodes. Instead, programs are constructed as **Transfluxors**—vocalized words defined by their synthesizer parameters and physical wave modulations.

```
                  +-----------------------------------+
                  |        THE WHEELER JUMP           |  <-- Setting Pulse
                  |  (Initial transient 'j' phoneme)  |      (Initializes Flux)
                  +-----------------+-----------------+
                                    |
                                    v
                  +-----------------+-----------------+
                  |        SYNTHESIZER CORE           |  <-- Modulates small aperture
                  | (Verlet stiffness / Formant shape)|      (Non-destructive read)
                  +-----------------+-----------------+
                                    |
                                    v
                  +-----------------+-----------------+
                  |      ABI ACCESSORS / GATES        |  <-- Register shifts
                  |    (NPN Black Gate/PNP Red Gate)  |      (Acoustic Execution)
                  +-----------------------------------+
```

---

## 1. Core Principles

Unlike traditional computing architectures that compile symbolic text strings into machine instructions, the **Auncient** Dysnomia VM operates directly on physical wave phenomena. 

1.  **Synthesizer Composition over Letters:** A word has no spelling. Its semantic structure is defined by an array of acoustic parameters: carrier frequency, formant filter resonances, Verlet membrane tensions, and Lissajous phase rotations.
2.  **Acoustic Execution:** Code execution occurs when these sound waves propagate through virtual hardware. Hardware gates (such as the NPN Black Gate and PNP Red Gate) react to frequency thresholds and envelope decay curves, modifying CPU registers.
3.  **Non-Destructive Read-out:** The transfluxor model allows continuous, non-destructive monitoring of active execution states by driving an interrogation wave through the core without clearing the initialized flux.
4.  **Non-Human Acoustic Envelope:** These acoustic words are optimized purely for register and hardware gate modulation, not for human language or speech. The frequency composition can extend into ultrasonic or subsonic bands, or consist of complex phase-interfered waveforms and mathematical noise profiles without needing to produce meaningful human-audible sounds.



---

## 2. The Wheeler Jump (The Word Delimiter)

Every Transfluxor word must begin with a **Wheeler Jump** (represented acoustically by the `'j'` phoneme attack transient).

*   **The Setting Pulse:** The Wheeler Jump acts as a high-voltage setting pulse applied to the transfluxor's large aperture. This pulse saturates the core's magnetic boundary, initializing the local flux loop.
*   **State Initialization:** The jump clears previous decay drift, resets the vactrol coil voltages back to the `ZENER_CLAMP_VOLTAGE` ($5.0\text{V}$), and sets the baseline frequency for the word's body.
*   **Boundary Gating:** A word cannot be resolved or parsed unless it is preceded by this high-energy attack transient.

---

## 3. Harmonic Synthesis Parameters (Semantic Mapping)

The semantic meaning of a Transfluxor word is mapped directly to physical sound properties:

| Synthesis Parameter | Mathematical Function | Operational Meaning |
|---|---|---|
| **Base Frequency ($f_0$)** | Determines the root pitch of the voice. | Directs register address targeting on the shared bus. |
| **Formant Resonances ($F_1, F_2$)** | Modulates vocal tract vocal shape. | Resolves context privilege levels and access parameters. |
| **Verlet Membrane Tension** | Governs mechanical envelope decay ($c_1, c_2$). | Modulates execution latency and clock-gating intervals. |
| **Lissajous Phase Offset ($\phi_w$)** | Governs the orbital X/Y/Z phase rotations. | Sets the target checksum and verification registers. |

---

## 4. Hardware Gating Interface (ABI Execution)

Execution is driven by passing the synthesized sound wave through virtual optoelectronic and semiconductor junctions:

*   **Vactrol Coils:** The audio amplitude controls the LED brightness of simulated optocouplers. The LDR lag acts as a temporal low-pass filter, smoothing control voltage sweeps to prevent transition jitter.
*   **Clock Gating:** If a Transfluxor's signal envelope decays below the $0.01$ threshold, the clock gate closes. This mutes processing ticks for the active voice, conserving system cycles.
*   **Transistor Gates:**
    *   **NPN Black Gate (Voiced):** Conduction through this gate represents voiced excitation (sound pressure), enabling register writes.
    *   **PNP Red Gate (Unvoiced):** Conduction through this gate represents unvoiced friction noise, routing execution to diagnostic subroutines.

---

## 5. Summary of Operation

To compile and run a program under the Acoustic Grammar:
1.  The compiler outputs a sequence of frequency-formant sweep coordinates instead of an assembly text file.
2.  The loader executes a Wheeler Jump, setting the initial state of the virtual transfluxor.
3.  The synthesizer plays the wave sweeps, driving the vactrol coils and transistor gates.
4.  The system transitions its CPU registers dynamically in response to the sound, completing execution when the audio envelope decays.
