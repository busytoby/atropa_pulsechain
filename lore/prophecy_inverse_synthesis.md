# 🔮 Inverse Prophecy Synthesis Design Plan

This document details the implementation strategy for the **Inverse Prophecy Synthesis** system. Inspired by early physical modeling synthesizer calibrations (such as the Korg Prophecy), this system maps target physical characteristics (like waveform spikiness or distortion) back to the genetic inputs (DNA parameters) of our simulated Wien-Bridge, Transducer, and Atoom-versterker engines.

---

## 1. Mathematical Objective & Nearest-Neighbor Search

Given a target feature vector $\vec{y}_{target} \in \mathbb{R}^k$ (containing target Crest Factor and Crossover Distortion metrics), our objective is to find the input gene configuration $\vec{g} \in [0.0, 1.0]^8$ that minimizes the Euclidean distance in feature space:

$$\text{arg min}_{\vec{g}} \left\| \vec{y}(\vec{g}) - \vec{y}_{target} \right\|_2^2$$

Since the physical equations governing the Wien-Bridge NTC thermal dynamics and transistor crossover leakage are highly non-linear, we pre-run a grid search to generate a **Prophecy Response Bank** dataset. We then perform a $k$-nearest neighbor ($k$-NN) search across this dataset.

---

## 2. Response Bank Binary Layout (`prophecy_response.bin`)

To allow fast memory-mapped file operations in both C and Yul, we store the sweep results in a contiguous binary array of fixed-size records:

| Field Offset | Data Type | Field Name | Description |
|---|---|---|---|
| `0x00 - 0x1F` | `float[8]` | `genes` | The 8 normalized DNA genes |
| `0x20 - 0x23` | `float` | `crest_factor` | Measured output Crest Factor |
| `0x24 - 0x27` | `float` | `crossover_dist` | Measured crossover distortion count |
| `0x28 - 0x2B` | `float` | `v_out_peak` | Measured peak output voltage |

Each record is exactly `44 bytes` wide.

---

## 3. Implementation Steps

1.  **Create [tsfi_prophecy.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_prophecy.h):** Interface declaring:
    *   `tsfi_prophecy_generate_bank(const char *filepath)`: Runs parameter sweeps and writes the binary file.
    *   `tsfi_prophecy_inverse_lookup(const char *filepath, float target_crest, float target_dist, TsfiBiotikaDna *out_dna)`: Reads the file and executes a nearest-neighbor search.
2.  **Create [tsfi_prophecy.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_prophecy.c):** The implementation loops, file I/O operations, and distance math.
3.  **Add [test_prophecy_synthesis.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_prophecy_synthesis.c):** Verification harness running the sweep, generating `prophecy_response.bin`, executing an inverse lookup, and verifying that the found genes produce a signal within $5\%$ tolerance of the target.
