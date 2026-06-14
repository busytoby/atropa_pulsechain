# Elektor/Elektuur Issue #8: Noise Generators, RFMs, & Compander Dynamics

This document provides a detailed technical analysis of the engineering and conceptual frameworks presented in *Elektor* Issue #8 (September 1977), focusing on Part 4 of the **Elektor Formant** series: the Noise Generator, Resonance Filter Module (RFM), and the **Compander** (Compressor/Expander) dynamic range adaptation system.

---

## 1. Key Articles & Architectural Alignment

| Elektor Issue #8 Concept | Original 1977 Technology | TSFi2 Target Subsystem | Implementation Translation |
| :--- | :--- | :--- | :--- |
| **Noise Generator** | White and pink noise source utilizing reverse-biased transistor junction noise. | **Excitation Source** | Pseudo-random noise generation algorithm in JavaScript for filter excitation. |
| **Resonance Filter Module** (RFM) | Parallel bandpass filter bank shaping vowel and formant resonances. | **Vowel Filter Bank** | Three SVF bandpass filters running in parallel in `studio.html`. |
| **Compander System** | Dynamic range compression (NE570) and expansion to maximize signal-to-noise ratio. | **Pre/Post Companding Loop** | Closed-loop dynamic compressor (pre-filter) and expander (post-filter) in `studio.html`. |

---

## 2. The Compander System: Pre-Filter Compression & Post-Filter Expansion

In Issue #8, the Compander (Compressor/Expander) was detailed as a means of processing dynamic signals:
*   **Compression (Pre-Filter)**: Reduces the dynamic range of the incoming signal, keeping peak levels below the threshold of sharp clipping.
*   **Expansion (Post-Filter)**: Restores the original dynamic range of the signal, magnifying the quiet parts and compensating for the compression.

By compressing the excitation signal *before* the resonant filter, we prevent clipping when the filter resonance rings. Expanding the signal *after* the filter restores dynamic contrast without re-introducing distortion:

```
                  Companding Architecture
                  
    Input ──► [ Compressor ] ──► [ Formant Filter ] ──► [ Expander ] ──► Output
                   ▲                                        ▲
                   └─────── Gain Control (Envelopes) ───────┘
```

---

## 3. Web Implementation of the Issue #8 Compander

In the Synthesis Studio dashboard (`frontend/studio.html`), we implement the Companding logic within the simulation step loop.

### A. Pre-Filter Compression
We track input envelope power using a fast-attack, slow-release window. The compression gain ($G_c$) scales down the input if it exceeds the limit:
$$V_{\text{compressed}} = V_{\text{input}} \times G_c$$

### B. Post-Filter Expansion
After processing the sample through the state-variable filter, the output is scaled back by the reciprocal of the compression gain:
$$V_{\text{output}} = V_{\text{filtered}} \times \frac{1}{G_c}$$

This preserves the tonal dynamics of the resonant sweeps while maintaining an absolute peak ceiling that prevents clipping in the DAC stage.

---

## 4. Conclusion

By integrating the Companding architecture from Elektor Issue #8, we secure the Synthesis Studio against clipping. Pre-filter compression prevents high-amplitude input peaks from driving the formant filter into saturation, while post-filter expansion preserves clean, punchy dynamics across the output waveform.
