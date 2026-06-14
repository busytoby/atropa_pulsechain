# Spectral Analysis Report: TSFi2 Dual YI Harmonizer Melody

An in-depth investigation of the physical modeling characteristics of the DeForest Audion triode curves (Tubular Type T vs. Flat-Plate Type S) and the resulting harmonic structures, total harmonic distortion (THD), and intermodulation distortion (IMD) in `harmonizer_melody.wav`.

---

## 1. Executive Summary

A spectral analysis was performed on `/home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/harmonizer_melody.wav`. The file contains 4 sequential perfect fifth note blocks, each 0.5 seconds in duration. By leveraging physical simulations of the individual valve stages, we evaluated the isolated behavior of the **Type T (Tubular)** and **Type S (Flat-Plate)** triodes.

### Key Findings:
- **Perfect Fifth Accuracy**: The synthesized notes match the C4+G4, E4+B4, F4+C5, and G4+D5 frequencies with high precision, maintaining the exact $3:2$ mathematical frequency ratio.
- **Harmonic Distortion Profiles**: 
  - **Type T (Tubular)**: Highly linear, low-distortion profile (THD ~3.0% to 3.4%) with a massive dominance of even-order harmonics (Even-to-Odd power ratio of **~38 to 40**). This creates a warm, classic triode signature.
  - **Type S (Flat-Plate)**: Highly nonlinear, high-distortion profile (THD ~24.7% to 26.3%) with a significant mix of both even and odd harmonics (Even-to-Odd power ratio of **~3.4 to 3.7**).
- **Intermodulation Products**: The cross-modulation blending term ($0.05 \cdot ac_1 \cdot ac_2$) introduces measurable sidebands at $|f_1 \pm f_2|$ and $2f_1 \pm f_2$. Due to the $3:2$ ratio of the perfect fifth, multiple harmonics and IMD components overlap precisely (e.g., $3f_1 = 2f_2$), thickening the acoustic texture.

---

## 2. Audio File Specifications

The file parameters of `harmonizer_melody.wav` were verified using Python's `wave` module:
- **Channels**: 1 (Mono)
- **Sample Rate**: 44,100 Hz
- **Bit Depth**: 16-bit PCM
- **Total Duration**: 2.0 seconds (4 blocks $\times$ 0.5s)
- **Total Samples**: 88,200 samples

---

## 3. Note Block Frequency Verification

The frequency spectra of the 4 synthesized note blocks were analyzed using a Hann-windowed Fast Fourier Transform (FFT). The table below compares the expected and measured fundamental frequencies:

| Block | Note Pair | Expected $f_1$ (Hz) | Measured $f_1$ (Hz) | Expected $f_2$ (Hz) | Measured $f_2$ (Hz) | Ratio ($f_2 / f_1$) | Status |
| :---: | :---: | :---: | :---: | :---: | :---: | :---: | :---: |
| **0** | C4 + G4 | 261.63 | 262.00 | 392.00 | 392.00 | 1.4962 | **Verified** |
| **1** | E4 + B4 | 329.63 | 330.00 | 493.88 | 494.00 | 1.4970 | **Verified** |
| **2** | F4 + C5 | 349.23 | 350.00 | 523.25 | 524.00 | 1.4971 | **Verified** |
| **3** | G4 + D5 | 392.00 | 392.00 | 587.33 | 588.00 | 1.5000 | **Verified** |

> [!NOTE]
> The small deviations from exact note frequencies are due to the discrete frequency bin spacing of the FFT ($\Delta f = 2 \text{ Hz}$ for $N = 22,050$ samples at $f_s = 44,100 \text{ Hz}$).

---

## 4. Valve Curve Comparison: Tubular vs. Flat-Plate

Because the two tones are mixed in the final audio file, analyzing the raw WAV file directly reports combined harmonic and IMD overlaps. To isolate the triode transfer curves, we simulated the individual valve circuits using the parameters from `test_harmonizer_melody.c`:

- **Valve 1 (Type T Tubular)**: $\mu = 90.0$, $K = 2 \cdot 10^{-5}$, $V_{bias} = -1.2\text{V}$, $R_{plate} = 120\text{k}\Omega$, $\text{geom\_scale} = 0.85$, $\eta = -0.2$.
- **Valve 2 (Type S Flat-Plate)**: $\mu = 110.0$, $K = 2.5 \cdot 10^{-5}$, $V_{bias} = -1.6\text{V}$, $R_{plate} = 80\text{k}\Omega$, $\text{geom\_scale} = 1.0$, $\eta = 0.2$.

### Isolated Harmonic Performance (Block 0: C4 + G4)

```mermaid
gantt
    title Harmonic Distortion Profiles (THD)
    dateFormat  X
    axisFormat %s
    section Tubular Type T
    THD: 3.01% : 0, 3
    section Flat-Plate Type S
    THD: 24.90% : 0, 25
```

#### Detailed Amplitude Spectra (Isolated)

| Parameter | Valve 1 (Tubular / $f_1 = 261.63$ Hz) | Valve 2 (Flat-Plate / $f_2 = 392.00$ Hz) |
| :--- | :---: | :---: |
| **Fundamental Amp** | 0.563355 | 0.394486 |
| **2nd Harmonic Amp** | 0.016974 | 0.094737 |
| **3rd Harmonic Amp** | 0.000423 | 0.024836 |
| **4th Harmonic Amp** | 0.000010 | 0.001811 |
| **5th Harmonic Amp** | 0.000000 | 0.007090 |
| **True THD** | **3.0139%** | **24.8959%** |
| **Even-to-Odd Power Ratio** | **40.1250** | **3.6686** |

### Physical Interpretation:
1. **The Tubular Advantage (Type T)**: By concentrating the electrostatic field logarithmically near the axial filament (modeled via `geom_scale = 0.85`), the triode functions with high linearity. It produces negligible odd-order harmonics. The second harmonic dominates the distortion spectrum by a factor of 40 over the third harmonic, imparting a clean, warm tube tone.
2. **The Flat-Plate Saturation (Type S)**: Operating at a full geometry scale (`geom_scale = 1.0`) and with positive dielectric compression (`eta = 0.2`), the flat-plate valve is driven heavily into asymmetric grid clipping. This generates severe harmonic distortion (~25% THD) and outputs substantial amounts of third and fifth harmonics, producing a brighter, more aggressive timbre.

---

## 5. Cross-Modulation and Intermodulation Distortion (IMD)

The bijective blending term `blended = ac1 + ac2 + 0.05 * ac1 * ac2` creates intermodulation products. The frequencies and amplitudes of these sidebands for Block 0 are detailed below:

| IMD Component | Expected Freq (Hz) | Measured Freq (Hz) | Amplitude | Overlapping Component |
| :---: | :---: | :---: | :---: | :---: |
| $|f_1 - f_2|$ | 130.37 | 130.00 | 0.002778 | $2f_1 - f_2$ (131.26 Hz) |
| $f_1 + f_2$ | 653.63 | 654.00 | 0.002708 | None |
| $2f_2 - f_1$ | 522.37 | 524.00 | 0.003686 | $2f_1$ (523.26 Hz) |
| $2f_1 + f_2$ | 915.26 | 914.00 | 0.000104 | None |
| $2f_2 + f_1$ | 1045.63 | 1046.00 | 0.000647 | $4f_1$ (1046.52 Hz) |

### Overlap Analysis in Perfect Fifths:
Due to the mathematical frequency relationship $f_2 \approx 1.5 \cdot f_1$, several harmonics and IMD terms coincide:
- **Consonance Reinforcement**: The third harmonic of the fundamental ($3f_1 = 784.89\text{ Hz}$) aligns perfectly with the second harmonic of the fifth ($2f_2 = 784.00\text{ Hz}$). This reinforces the octave of the fifth.
- **Sideband Overlap**: The second-order difference product $|f_1 - f_2|$ (130.37 Hz) overlaps with the third-order intermodulation product $2f_1 - f_2$ (131.26 Hz), consolidating energy in the low-frequency sub-bass register.
- **Harmonic Overlap**: The IM product $2f_2 - f_1$ (522.37 Hz) lands directly on top of the fundamental's second harmonic $2f_1$ (523.26 Hz), modulating the perceived "warmth" of the lower note.

---

## 6. Conclusion

The dual YI harmonizer design successfully utilizes the distinct electrostatic profiles of the tubular and flat-plate Audion triodes. By routing the lower fundamental note through the linear, even-harmonic-heavy tubular stage, and the fifth note through the heavily saturated, rich flat-plate stage, the system achieves a balanced blend. The $5\%$ cross-modulation term acts as an analog-like summing mixer, generating cohesive intermodulation sidebands that thicken the overall perfect-fifth harmony.
