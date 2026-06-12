# Computer Design Magazine (Issue #4 - March 1963) Architectural Analysis

This report reviews the core technical concepts of March 1963-era hardware engineering featured in **Computer Design Magazine (Vol. 2, No. 3)**, and documents the mapping of these systems to the **TSFi2** digital simulation pipeline.

---

## 1. Core Historical Features

### Binary Shaft Encoders & Gray Code
* **The Transition Glitch Problem:** In early optical and brush-contact shaft encoders, rotating the shaft between consecutive values in standard binary (e.g., $0111_2 \to 1000_2$) caused major errors because mechanical sensors do not align perfectly. Intermediate readout states (e.g. $1111_2$) would read incorrectly.
* **The Gray Code Solution:** Frank Gray (Bell Labs) patented a unit-distance binary code where consecutive numbers differ by exactly one bit.
* **Logic Scheme:**
  ```
  Shaft Angle (Theta) -> Angular Quantization (Standard Binary) -> Gray Code Converter -> Glitch-Free Sensor Readout
  ```

---

## 2. TSFi2 Integration Architecture

To emulate a binary shaft encoder with Gray code mapping, we introduce:
1. **[tsfi_shaft_encoder.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_shaft_encoder.h):** Interface declaring encoder quantization levels, binary-to-Gray, and Gray-to-binary translation routines.
2. **[tsfi_shaft_encoder.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_shaft_encoder.c):** Digital implementation of angular quantization, encoding, and decoding logic.
3. **[test_shaft_encoder.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_shaft_encoder.c):** Verification test suite ensuring unit-distance hamming properties and bidirectional conversion.

### Mathematical Conversions
* **Angular Quantization:**
  Given shaft angle $\theta \in [0, 2\pi)$ and resolution $N$ bits:
  $$\text{Binary } B = \text{floor}\left( \frac{\theta}{2\pi} \cdot 2^N \right) \bmod 2^N$$
* **Binary to Gray Translation:**
  $$G = B \oplus (B \gg 1)$$
* **Gray to Binary Translation:**
  For bit index $i$ from $N-1$ down to $0$:
  $$B_i = \begin{cases} G_i & \text{if } i = N-1 \\ G_i \oplus B_{i+1} & \text{otherwise} \end{cases}$$

---

## 3. Verification & Memory Sanitation
The encoder simulator is verified under the standard memory check guidelines of `LauMemory`.
