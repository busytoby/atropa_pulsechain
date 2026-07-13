# Auncient VM: Fourier Operator & Audio Modulation

This document details the mathematical design, routing architecture, and verification of the **Auncient** Fourier Operator system implemented across the Helmholtz Linux init daemon and the host ALSA synthesizer thunks.

## 1. System Overview
The system acts as a complete, closed-loop **Fourier Transform Pair** mapped directly between guest virtual storage and host output devices. It completely bypasses physical acoustics and spatial distance attenuation:
* **Forward Transform (Analysis):** Occurs inside the guest Helmholtz Linux environment (`zmm_init.c`), mapping spatial coordinate registers to discrete frequency coefficients.
* **Inverse Transform (Synthesis):** Occurs on the host hypervisor thread (`tsfi_alsa_synth.c`), synthesizing the frequency coefficients back into continuous time-domain waveforms.

```
+--------------------------------------------------------+
|                      GUEST VM                          |
|  [Spatial Coordinates] -> Forward DFT -> [Peak Bins]    |
+------------------------------------------+-------------+
                                           |
                                   Shared State File
                                   (tmp/audio_freq)
                                           |
+------------------------------------------v-------------+
|                      HOST HYPERVISOR                   |
|  [Peak Frequencies] -> Inverse DFT -> [ALSA Sine Wave] |
+--------------------------------------------------------+
```

## 2. Forward DFT Analysis
During guest startup, the init daemon executes a Discrete Fourier Transform (DFT) over a mock spatial coordinate input:
$$\text{Input}(n) = \sin\left(\frac{2\pi \cdot 2n}{N}\right) + 0.5\sin\left(\frac{2\pi \cdot 5n}{N}\right)$$
The forward DFT resolves the magnitude spectrum:
$$X(k) = \sum_{n=0}^{N-1} \text{Input}(n) \cdot e^{-i 2\pi k n / N}$$
Active peak bins (Bin 2 and Bin 5) are identified and mapped to target tones ($f = k \times 220.0$ Hz), yielding **440.0 Hz** and **1100.0 Hz**.

## 3. Host ALSA Synthesis (Inverse DFT)
The guest writes these active peak frequencies to a shared rootfs state file (`/tmp/audio_freq`). The host **ALSA Sound Synthesis Modulator** reads these values and updates the wave phase generator:
$$\text{Wave}(t) = \sin(\phi)$$
$$\phi_{t+1} = \phi_t + \frac{2\pi \cdot f}{\text{SampleRate}}$$
This modulates the sound generator thread in real-time based on guest-calculated spatial frequencies.

## 4. Waveform Visualization
To verify wave shape alignment, the guest prints a vertical ASCII plotter of the superposition signal directly to the boot logs:
```
[INIT] Reconstructed Waveform ASCII Plot (Superposition of bins 2 & 5):
  00:                *
  01:                           *
  02:                      *
...
```

## 5. Verification Targets
* **Dynamic Gas Limit:** `tests/test_auncient_dynamic_gas`
* **Fourier Loopback:** `tests/test_auncient_fourier_alsa_feedback`
* **Containerized Boot:** `tests/test_auncient_boot_transform_c`

## 6. The ALSA Coaxial Pipe Analogy
The connection interface to the ALSA modulator acts as a digital coaxial cable:
* **High-Fidelity Shielding:** Insulates the raw frequency parameters from host thread interference, preventing environment-induced jitter.
* **Point-to-Point Pipeline:** Directly maps the guest's analyzed Fourier peak variables to the host sound generator without format overhead or intermediary conversion stages, acting as a clean, physical patch channel.
