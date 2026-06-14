# Elektor/Elektuur Issue #19: Plumbicon Photoconductor Lag Reduction, Veder-Bias, and Instantaneous Opto-Compression

This document provides a detailed technical analysis of the engineering and physical principles presented in *Elektor (Elektronika Wereld)* Issue #19 (April 1964), focusing on how lead-oxide (PbO) camera tube lag reduction translates to eliminating attack lag in our **TSFi2 Synthesis Studio** opto-compressors.

---

## 1. Plumbicon Photoconductive Lag Reduction (Veder-Bias)

Issue #19 celebrates the awarding of the prestigious **Vederprijs 1964** to Dr. E.F. de Haan and Dr. H. Bruining for their work on the **Plumbicon television camera tube**. 

### A. The Physics of Photo-Lag
In lead-oxide (PbO) targets, charge carriers (electrons/holes) get trapped in shallow energy states. At low light levels, the slow thermal release of these trapped carriers causes significant **image lag** (ghosting/trailing behind moving objects).
To solve this, inventors introduced a constant **background light bias**:
* Keeping the shallow traps continuously filled.
* Ensuring that any new photo-signal immediately translates to conduction current without lag.

### B. Application: Optocoupler Veder-Bias
In our Gallium Phosphide (GaP) opto-compressor, when starting from absolute darkness ($V_{in} = 0$), the LDR exhibits a slow initial resistance change (attack lag), letting brief transient spikes leak through before the LED illuminates.

By implementing a virtual **Veder-Bias**:
- We inject a constant background illumination $L_{bias}$ into the optocoupler.
- This pre-biases the photoresistor's resistance slightly down from the dark state ($R_{dark}$ is shunted to $8.2\text{k}\Omega$ instead of $10\text{k}\Omega$).
- The moment a signal surge occurs, the opto-compressor reacts instantaneously with zero initial barrier delay.
- **Math**:
  $$L(t) = L_{bias} + \alpha \cdot \max(0, |V_{in}| - V_{forward})$$
  $$R_{LDR}(t) = R_{dark} \cdot e^{-\gamma \cdot L(t)}$$

---

## 2. Acoustic Amplifier Gain Distribution (Akoestische Geluidsversterkers)

The issue covers acoustic feedback and gain distribution. To prevent howling and clipping in multi-microphone setups, gain must be distributed logarithmically across stages, aligning with the opto-bias knee shape to prevent step distortion.
