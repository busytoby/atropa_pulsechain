# 🔌 QST Issues #1–#4: Further Hardware & Code Implementation Concepts

This document explores technical concepts introduced in the first four issues of *QST* Magazine (December 1915 – March 1916) and defines how they can be implemented inside the **TSFi2** simulation and audio synthesis environment.

---

## 1. Candidate Hardware Concepts

| QST Issue | Article / Concept | Historical Physics Detail | TSFi2 Code Implementation Translation |
| :--- | :--- | :--- | :--- |
| **Issue #1** (Dec 1915) | **"Pictured Electro-Magnetic Waves"** (Tuska) | Early visualizations of electromagnetic wave fronts and field intensity. | RDNA4 compute shaders generating dynamic wave-propagation fields in Vulkan VRAM (`VGP`). |
| **Issue #2** (Jan 1916) | **"The Oscillating Audion"** | Regenerative feedback loop where the plate AC output is fed back to the grid to trigger self-oscillation. | A feedback factor $\beta$ in the AVX-512 valve processing loop: $Vg_{\text{feedback}} = Vg_{\text{in}} + \beta \cdot Vp_{\text{AC}}(t-1)$. |
| **Issue #3** (Feb 1916) | **"500-Cycle High-Pitch copy"** | Advantage of using high-pitched spark transmitters (500 Hz alternator) to cut through atmospheric static. | A bandpass DSP filter centered at 500 Hz to simulate the selective copying efficiency of historical spark receptions. |
| **Issue #4** (Mar 1916) | **"Audion Tuner Battery Box"** (Cole) | Step-by-step series cell battery regulator providing fine tuning steps ($\pm 1.5\text{ V}$) for the plate. | A micro-voltage bias regulator modifying the supply voltage $V_p$ within the physical valve state. |

---

## 2. Deep-Dive & Implementation Plan

### A. The Oscillating Audion (Regenerative Feedback)
By feeding a portion of the plate voltage output back into the grid, the Audion functions as both an amplifier and an oscillator (a precursor to Armstrong's superheterodyne receiver).
* **Mathematical Model:**
  $$Vg_{\text{total}}(t) = Vg_{\text{in}}(t) + Vg_{\text{bias}} + \beta \cdot \left(Vp(t-1) - Vp_{\text{idle}}\right)$$
  Where $\beta$ is the feedback coefficient. If $\beta$ exceeds a critical threshold, the loop becomes unstable and begins self-oscillation, creating a beat frequency (heterodyne) against incoming continuous waves.

### B. 500-Cycle High-Pitch Copying
Amateurs found that 500 Hz tone spark transmissions were far easier to copy through high-energy broadband static (lightning, static electricity).
* **Implementation:** Add a 2nd-order resonant bandpass filter (Q factor of 10) centered at 500 Hz to show the SNR improvement when static noise is injected into the valve grid.
