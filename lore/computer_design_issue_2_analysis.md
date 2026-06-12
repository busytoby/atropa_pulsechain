# Computer Design Magazine (Issue #2 - January 1963) Architectural Analysis

This report reviews the core technical concepts of January 1963-era hardware engineering featured in **Computer Design Magazine (Vol. 2, No. 1)**, and documents the mapping of these systems to the **TSFi2** digital simulation pipeline.

---

## 1. Core Historical Features

### Magnetostrictive Delay Line Memory
* **The Storage Paradigm:** Prior to affordable core arrays or silicon registers, airborne computers utilized sonic or torsional waves propagating along a nickel-iron wire to store digital bits serially.
* **Recirculating Architecture:** Bits are written by magnetostrictive transducers (electrical to mechanical pulses), travel through the acoustic medium, are received at the output, amplified, reshaped, and gated back into the transmitter to maintain loop memory.
* **Signal Topology:**
  ```
  Write Data --> OR Gate --> Transducer (Input) --> Acoustic Wire (Delay: Tau) --> Transducer (Output)
                   ^                                                                  |
                   +------- Recirculation Gate (Read/Write Control) <------- Detector / Reshaper
  ```

---

## 2. TSFi2 Integration Architecture

To emulate the recirculating delay line memory loop, we introduce:
1. **[tsfi_delay_line.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_delay_line.h):** Interface declaring the delay buffer, bit index, read/write gates, and delay times.
2. **[tsfi_delay_line.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_delay_line.c):** Digital emulation of serial acoustic propagation, amplifier signal attenuation, and gating controls.
3. **[test_delay_line.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_delay_line.c):** Verification test suite mapping bit pattern writes and multi-loop recirculation.

### Mathematical Delay Model
Let the delay line hold $N$ bits representing a total delay of $\tau = N \cdot T_{bit}$.
* **Recirculating Equation:**
  The output bit $y(t)$ at the detector is the input bit delayed by $\tau$:
  $$y(t) = x(t - \tau)$$
* **Input gating logic (Write Enable $WE$, Recirculate Enable $RE$):**
  $$x(t) = (WE \cdot \text{Data}_{in}) \text{ OR } (RE \cdot \text{Reshaped}(y(t)))$$
  where $\text{Reshaped}(y)$ amplifies and digitizes the signal back to clean $0$ or $1$ levels.

---

## 3. Verification & Memory Sanitation
The memory loop simulator is verified under the standard memory tracking suite of `LauMemory`.
