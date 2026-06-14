# 📰 QST Magazine Issue #7 (June 1916) & TSFi2 Distributed Inductive Resonance

This document details the historical and technical findings of **QST Issue #7** (June 1916, Vol. 1, No. 7), focusing on the first historical origin of the **QSL card** and the engineering solutions for the **"dead-end effect"** in tuning coil distributed capacity, showing their mapping to the **TSFi2** resonant networking layers.

---

## 1. Key Articles & Architectural Alignment

| QST Issue #7 Article / Concept | Historical Technical Detail | TSFi2 Implementation Translation |
| :--- | :--- | :--- |
| **First QSL Card Mention** (Edward Andrews, 3TW) | A published letter detailing a signal verification card exchanged between 3TW and 8VX, starting the QSL verification card tradition. | On-chain signature verification and transaction receipt generation logs modeled in `tsfi_merkle.c`. |
| **"Distributed Capacity" & "Dead-End Effect"** | Unused turns of tuning coils absorb energy and shift resonant frequency due to stray/distributed capacity, requiring "dead-end switches" to isolate unused windings. | Resonant bandpass filter optimizations and coil inductance coupling calculations in [tsfi_resonance.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_resonance.c). |
| **"Short-Wave Condensers"** | Variable tuning capacitors in series or shunt configuration to adjust short-wave resonant peaks. | Dynamic parameter calibration matrices within the synthesis core. |

---

## 2. Technical Deep-Dive: Distributed Capacity & Dead-End Isolation

In early receiver design, tuning coils were tapped at multiple points to select different wavelength ranges. However, the unused portion of the coil remained magnetically coupled and possessed parasitic capacity between the turns:
* **The Dead-End Effect:** The unused portion acts as an auxiliary tank circuit. It absorbs electromagnetic energy from the active circuit, causing a severe drop in receiver sensitivity (high loss, $\Phi > 0$) and shifting the tuning peaks.
* **Dead-End Switch Mitigation:** A multi-blade switch was used to not only select the active tap but to completely open-circuit or short-out the unused segments, preventing resonant absorption.

### TSFi2 Resonant Analogy
Within the **Synthesis Resonant network** (`tsfi_resonance.c`), we simulate multi-stage inductors:
* If a multi-stage inductor has unused segments, we calculate the parasitic distributed capacitance coupling coefficient $k_{\text{parasitic}}$ between the active and inactive sections.
* Implementing a virtual "Dead-End switch" cuts this coupling coefficient to zero ($k_{\text{parasitic}} = 0$), restoring the zero-loss ($\Phi = 0$) invariant and increasing peak output gain.
