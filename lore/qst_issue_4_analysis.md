# 📰 QST Magazine Issue #4 (March 1916) & TSFi2 Thermionic Valve Synthesis Alignment

This document details the historical and technical findings of **QST Issue #4** (March 1916, Vol. 1, No. 4), focusing on A. B. Cole's seminal article *"Practical Pointers on the Audion"*, and maps these findings directly to the physical vacuum tube models simulated in the **TSFi2** audio synthesis framework.

---

## 1. Key Articles & Technical Translation

| QST Issue #4 Article / Concept | Historical Technical Detail | TSFi2 Implementation Translation |
| :--- | :--- | :--- |
| **"Practical Pointers on the Audion"** (A. B. Cole) | Audion Bulbs testing, sensitivity variation (X vs S grades), common operator errors, critical voltages, two operating points, and fine-tuning potentiometer grids. | Emulated grid dielectric modulation and soft-vacuum gas ionization current surges in [tsfi_valve.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_valve.c). |
| **"Practical Relaying"** | Early spark-gap relay network topology and timing propagation across stations. | Network transmission latency models and queue management in `tsfi_comm.c`. |
| **"Ann Arbor Y.M.C.A. Radio Club"** | Amateur station reports operating with early spark-transmitters and high-tension transformers. | Simulation of transformer distortion and high-voltage supply sag under load. |

---

## 2. Deep-Dive: A. B. Cole's Audion Technical Observations

A. B. Cole, the Sales Manager for the De Forest Radio Telephone & Telegraph Company, highlighted critical operating conditions of early soft-vacuum triodes:

### A. Testing & Sensitivity Variations (Grades "X" and "S")
* **Historical Observation:** Bulbs were tested in a double-detector circuit against a standard crystal detector under real receiving conditions. Throwing circuits out of tune was noted as incorrect because the Audion is a **potentially operated detector** (voltage-controlled, high impedance input). Only 1% to 20% of bulbs qualified as "X" (extra sensitive).
* **TSFi2 Mapping:** The simulated grid bias $V_g$ behaves as a pure voltage-controlled gate. Input impedance is modeled as infinite at low frequencies, ensuring signal amplitude modulates permittivity without drawing grid current unless driven positive.

### B. Filament Temperature & "B" Battery Degradation
* **Historical Observation:** Operators often run filaments too bright when the "B" battery voltage drops below $3.5\text{ V}$ per cell, leading to premature burnout.
* **TSFi2 Mapping:** The simulation tracks the coupling of filament intensity ($A$ battery) and plate voltage ($B$ battery) through the Child's Law amplification factor $\mu$:
  $$\mu_{\text{active}} = \mu_0 \cdot (1 + \eta)$$

### C. The Two Critical Points & Reversing Connections
* **Historical Observation:** The Audion has two distinct tuning zones:
  1. Low $B$ voltage with normal filament heating.
  2. High $B$ voltage with lower filament intensity (requiring lighting/filament battery reversal). This second point yields longer tube life.
* **TSFi2 Mapping:** Permittivity $\epsilon(t)$ is modeled as a function of the signal:
  $$\epsilon(t) = 1.0 + \eta \cdot V_g(t)$$
  Reversing lighting battery terminals modulates the operating point from positive to negative phase-bias offsets.

### D. Blue Glow & Gas Ionization
* **Historical Observation:** Hissing and blue glow occur due to gas ionization. If allowed to continue, the vacuum increases (adsorption of gas ions into the glass/metal), raising the required operating voltage to $75\text{--}150\text{ V}$.
* **TSFi2 Mapping:** Ionization surge current is computed using a fast sigmoid approximation for plate voltage $V_p > V_{\text{ionization}}$:
  $$I_{p,\text{total}} = I_{p,\text{base}} \cdot \left(1.0 + \text{ion\_factor} \cdot \text{sigmoid}(V_p - V_{\text{ionization}})\right)$$

---

## 3. Physical Model Integration Plan

To implement the historical findings and expand our simulation to include the Koren Pentode Formula (AVX-512 vectorized) and finer battery regulation:

1. **Fine Battery Potential Regulation:** We will implement a fine step adjustment representing Cole's proposed "extra 3-cell battery box series tuner" to let users micro-adjust the operating point by $\pm 1.5\text{ V}$ increments.
2. **Koren Pentode Formula:** We will add support for the classic EL84 power pentode simulation, modeling the screen grid ($G_2$) current division and plate knee characteristics.
