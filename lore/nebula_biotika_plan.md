# BCD Nebula "Biotika" Evolution Generation Plan

This document outlines the design and integration path for evolving **BCD Nebula** synthesizer parameters using the TSFi2 genetic framework to synthesize the "Biotika Spike" audio signature (rapid transients, high crest factors, and sub-octave phase anomalies).

---

## 1. Genetic Representation (Nebula DNA)

Each individual patch in the Nebula genetic population is represented by an array of normalized genes $\vec{g} \in [0.0, 1.0]^8$, mapping directly to the physical parameters of the emulator:

| Gene Index | Parameter Name | Mapping Domain | Synthesizer Target |
|---|---|---|---|
| $g_0$ | Cutoff Base | $[100.0\text{ Hz}, 2500.0\text{ Hz}]$ | `cutoff_base` |
| $g_1$ | Resonance | $[0.0, 0.95]$ | `resonance` |
| $g_2$ | Compressor Alpha| $[0.0, 15.0]$ | `compressor_alpha` |
| $g_3$ | Fuzz Gain | $[1.0, 30.0]$ | `fuzz_gain` |
| $g_4$ | Fuzz Mix | $[0.0, 1.0]$ | `fuzz_mix` |
| $g_5$ | Octave Mix | $[0.0, 1.0]$ | `octave_mix` |
| $g_6$ | LFO Rate | $[0.2\text{ Hz}, 40.0\text{ Hz}]$ | `lfo_rate` |
| $g_7$ | LFO Depth | $[0.0, 1.0]$ | `lfo_depth` |

---

## 2. Genetic Operators & Crossover

We define the evolutionary operators inside `src/tsfi_nebula_genetic.c`:

* **Uniform Crossover:** Blends parent chromosomes $P_A$ and $P_B$ to produce child $C$:
  $$C_i = \beta_i P_{A,i} + (1.0 - \beta_i) P_{B,i}$$
  where $\beta_i \in [0, 1]$ is randomly selected per gene.
* **Gaussian Mutation:** Adds minor variations to genes at a rate $\eta$:
  $$g_i' = \text{clamp}(g_i + \mathcal{N}(0, \sigma^2), 0.0, 1.0)$$

---

## 3. Fitness Function: "Biotika Spikiness"

The objective function optimizes for a high-transient, aggressive "Biotika Spike" pattern by analyzing the processed output buffer:

1. **Crest Factor Optimization:** Maximizes the ratio of peak amplitude to RMS:
   $$\text{Fitness}_{\text{crest}} = \frac{V_{\text{peak}}}{V_{\text{RMS}}}$$
2. **Spectral Flux (High High-Frequency Transients):** Measures the rate of spectral changes in successive FFT bins.
3. **Octave sub-harmonic density:** Penalizes clean sine outputs to favor fuzz-divider square wave variations.

---

## 4. Implementation Steps

1. **Create [tsfi_nebula_genetic.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_nebula_genetic.h):** Interface declaring DNA parsing, crossovers, and mutations.
2. **Create [tsfi_nebula_genetic.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_nebula_genetic.c):** Implementation of operators and fitness calculators.
3. **Add [test_nebula_genetic.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_nebula_genetic.c):** Verification test running a mini genetic simulation (e.g. 50 generations) seeking to maximize the spikiness index.
