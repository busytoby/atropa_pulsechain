# Elektor/Elektuur Issue #13: Wien-Bridge Selectivity, Band-Select Decoders, and Anti-Resonance Clipping Traps

This document provides a detailed technical analysis of the engineering and physical principles presented in *Elektor (Electronica Wereld)* Issue #13 (September 1963), focusing on how Wien-Bridge selective tone decoders can act as precise notch filters to trap and extinguish clipping resonances in the **TSFi2 Synthesis Studio**.

---

## 1. Wien-Bridge Selective Tone Decoder (PTT Semafoon)

Issue #13 details the **Wien-Bridge Selective Tone Decoder** utilized in early PTT selective paging decoders. The core selective block consists of a Wien-Bridge bandpass network that acts as a feedback band-select filter with an exceptionally high quality factor ($Q$).

### A. The Wien-Bridge Transfer Function
The feedback network is balanced at a specific resonant frequency $f_0$:

$$f_0 = \frac{1}{2\pi \sqrt{R_1 C_1 R_2 C_2}}$$

When the input signal contains a tone matching $f_0$, the bridge achieves balance. This triggers a sharp peak envelope output from the selective amplifier stage, while blocking all other adjacent frequencies.

---

## 2. Application: The Wien-Bridge Dynamic Notch Trap

During sharp clipping overloads in formant synthesis, active filters enter sustained ringing oscillations at their resonant peaks. Applying a global volume reduction (like CCS attenuation) darkens the whole signal.

By applying the Wien-Bridge selective model:
- **Selective Tone Trapping**: We configure a virtual Wien-Bridge notch filter block placed in the output path of the formant filter.
- **Dynamic Center Tracking**: The Wien-Bridge notch center frequency $f_0$ is dynamically aligned to follow the formant filter's active center frequency ($F_1$).
- **Anti-Resonance Attenuation**: When a clipping warning triggers, the notch depth is instantly deepened, selectively trapping and extinguishing the high-Q ringing frequency.
- **Acoustic Result**: Keeps the rest of the vocal frequency spectrum open and bright, target-attenuating only the clipping limit cycles.
