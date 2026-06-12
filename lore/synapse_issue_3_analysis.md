# Synapse Electronic Music Magazine (Issue #3 - September/October 1976) Architectural Analysis

This report reviews the core technical concepts, historical synthesizer designs, and DIY projects featured in **Synapse Volume 1, No. 3 (September/October 1976)**, and documents the mapping of these systems to the **TSFi2** digital signal processing pipeline.

---

## 1. Core Historical Features

### Kraftwerk & Tom Oberheim Interviews
* **Kraftwerk's Studio Philosophy:** Reviewed early *Radio-Activity* era patch networks and custom electronic drum triggers.
* **Tom Oberheim's Expansion System:** Discussed the transition from standalone synthesizer modules (SEM) to polyphonic keyboard systems (using digital programmers to scan active keyboard gates).

### 7-Stage Frequency Divider
* **The DIY Objective:** Designed to "hot-rod" the Paia Gnome synthesizer, this circuit acts as a sub-harmonic generator using CMOS binary ripple counters (equivalent to CD4024).
* **Signal Path:**
  ```
  Input Audio -> Comparator (Schmitt Trigger) -> Binary Counter (7 stages: /2, /4, /8, /16, /32, /64, /128) -> Stage Mixer -> Output
  ```

---

## 2. TSFi2 Integration Architecture

To emulate the 7-stage CMOS frequency divider, we introduce:
1. **[tsfi_divider.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_divider.h):** State configuration for comparator thresholds and weighting coefficients.
2. **[tsfi_divider.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_divider.c):** Digital emulation of Schmitt Trigger comparator and stage division logic.
3. **[test_divider.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_divider.c):** Test suite mapping input audio waveforms to mixed sub-harmonic outputs.

### Mathematical Model
* **Schmitt Trigger State:**
  $$V_{gate}(t) = \begin{cases} 1 & \text{if } V_{in}(t) > V_{\text{high}} \\ 0 & \text{if } V_{in}(t) < V_{\text{low}} \\ V_{gate}(t-\Delta t) & \text{otherwise} \end{cases}$$
* **CMOS Stage Toggle:**
  For stage $k \in \{1 \dots 7\}$, state $S_k$ flips when stage $k-1$ drops from $1$ to $0$ (falling edge):
  $$S_k \leftarrow -S_k \quad \text{when } S_{k-1} \text{ falling edge}$$
* **Output Mix:**
  $$V_{out}(t) = \sum_{k=1}^7 w_k \cdot S_k(t)$$
  where $w_k$ is the relative stage weight coefficient.

---

## 3. Verification & Memory Sanitation
The emulator is checked for memory allocations and memory-alignment safety under the standard `LauMemory` test framework.
