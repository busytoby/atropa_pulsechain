# Computer Design Magazine (Issue #17 - August 1964) Architectural Analysis

This report reviews the core technical concepts of August 1964-era mixed-signal hardware engineering featured in **Computer Design Magazine (Vol. 3, No. 7)**, specifically focusing on **Successive Approximation Register (SAR) Analog-to-Digital Converters (ADCs)**, and documents its simulation mapping in **TSFi2**.

---

## 1. Core Historical Features

### Successive Approximation Register (SAR) ADC
* **Data Acquisition for Digital Systems:** Early industrial control and aerospace computers required converting analog transducer signals into digital binary words.
* **The Binary Search Algorithm:** The SAR ADC uses a single analog comparator and a Digital-to-Analog Converter (DAC) to perform a binary search. Starting from the MSB, it tests each bit, keeping it if the DAC output is lower than the input, or clearing it if the DAC output exceeds the input. An $N$-bit conversion completes in exactly $N$ clock cycles.
* **Mixed-Signal Constraints in 1964:**
  * **R-2R Ladder Tolerances:** Resistor mismatches in the DAC ladder lead to non-linearity (INL/DNL errors) and non-monotonic behavior.
  * **Comparator Offset & Hysteresis:** Thermal drift and offset voltages in the discrete transistor comparator limit resolution.

---

## 2. TSFi2 Integration Architecture

To model the physical mixed-signal behavior of a historical 8-bit SAR ADC (incorporating R-2R ladder tolerances, comparator offsets, and settling delay dynamics), we introduce:

1. **[tsfi_sar_adc.h](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/inc/tsfi_sar_adc.h):** Interface declaring the SAR ADC configuration parameters, DAC R-2R ladder offsets, and conversion step functions.
2. **[tsfi_sar_adc.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_sar_adc.c):** Physical model simulating the DAC voltage output with resistor errors, analog comparator decisions with offset, and clock cycle timing.
3. **[test_sar_adc.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_sar_adc.c):** Test suite verifying conversion accuracy under varying DAC tolerances and comparator offsets, checking monotonic performance, and ensuring zero memory leaks.

### Mathematical & Physical Model

#### 1. Mismatched R-2R DAC Output
The DAC output voltage $V_{dac}$ for an 8-bit digital word $D = [d_7 d_6 \dots d_0]$ factoring in resistor tolerance errors $\epsilon_i$ for each stage is:
$$V_{dac} = V_{ref} \sum_{i=0}^{7} d_i \cdot \frac{1 + \epsilon_i}{2^{8-i}}$$
Where $\epsilon_i \in [-tolerance, tolerance]$. If resistor tolerances are untrimmed ($\sim 5\%$), the output can become non-monotonic.

#### 2. Comparator Decision with Offset and Deadband
The comparator output $C_{out}$ is modeled as:
$$C_{out} = \begin{cases} 
1 & \text{if } V_{in} - V_{dac} - V_{offset} \ge \frac{\Delta V}{2} \\
0 & \text{if } V_{in} - V_{dac} - V_{offset} \le -\frac{\Delta V}{2} \\
\text{random}(0, 1) & \text{otherwise (within noise/deadband region)}
\end{cases}$$
Where $V_{offset}$ is the comparator offset voltage and $\Delta V$ is the resolution deadband.

---

## 3. Verification & Memory Sanitation
The SAR ADC simulator is fully verified under the standard memory check guidelines of `LauMemory` with zero unsealed leaks.
