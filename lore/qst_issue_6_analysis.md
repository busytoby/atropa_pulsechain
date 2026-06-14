# 📰 QST Magazine Issue #6 (May 1916) & TSFi2 Autodyne Demodulation

This document details the historical and technical findings of **QST Issue #6** (May 1916, Vol. 1, No. 6), focusing on C. D. Curtis and W. O. Horner's article *"The Audion as a Detector of Undamped Waves"*, and maps these findings directly to the **Autodyne Demodulation** models of the **TSFi2** VM.

---

## 1. Key Articles & Architectural Alignment

| QST Issue #6 Article / Concept | Historical Technical Detail | TSFi2 Implementation Translation |
| :--- | :--- | :--- |
| **"The Audion as a Detector of Undamped Waves"** (Curtis & Horner) | Describes utilizing the Audion to receive continuous waves (undamped signals) by causing the Audion itself to oscillate (heterodyning/feedback beats). This is historically termed **Autodyne Reception**. | Dynamic regenerative feedback loop ($\beta$ oscillation) producing a distinct beat tone when mixing a local oscillator with incoming RF in `tsfi_valve.c`. |

| **"The Spark Coils"** (Edgar Felix) | Technical pointers on spark coils, induction, and secondary spark discharge characteristics. | Modeling inductive decay curves and high-voltage coil saturation inside `tsfi_resonance.c`. |
| **"Portable Station SK"** | Operational review of mobile/portable amateur radio setups and power constraint limitations. | Dynamic power budgeting and battery/supply voltage sag simulations inside the VM core. |

---

## 2. Technical Deep-Dive: Autodyne Demodulation Principles

Prior to the discovery of autodyne reception (self-oscillation), detecting undamped (continuous wave) Morse code was challenging because the incoming signal had a constant frequency and produced no audible tone in the headphones (unlike the pulsed, musical tone of spark transmitters).

Curtis and Horner described using the Audion as an autodyne receiver:

* **The Beat Principle (Heterodyning):** The Audion is tuned to oscillate at a local frequency $f_1$ (via regenerative feedback $\beta$). When an incoming continuous wave frequency $f_2$ is received, the non-linear transfer characteristics of the Audion mix the two frequencies, producing an audible beat frequency:
  $$f_{\text{beat}} = |f_1 - f_2|$$
* **Non-Linear Mixing:** Because plate current $I_p$ is a non-linear function of grid voltage ($1.5$-power Child's Law), the product term of the local oscillation and incoming signal is naturally generated in the plate current:
  $$I_p \propto \left(V_{g,\text{local}} \cdot V_{g,\text{incoming}}\right)$$
  This generates sum and difference frequencies, with the low-frequency difference ($f_{\text{beat}}$) falling within the audible range ($500\text{--}1000\text{ Hz}$) of the Stromberg-Carlson headphones.

---

## 3. Physical Model Alignment

We have already implemented this regenerative feedback mechanism in [tsfi_valve.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_valve.c):
$$\text{prev\_vp} - \text{dc\_offset}$$
Which is fed back to the grid. In our tests, setting $\beta = 0.85$ triggers self-sustained oscillation, allowing the valve to serve as a local oscillator ($f_1$) to heterodyne incoming RF signals.
