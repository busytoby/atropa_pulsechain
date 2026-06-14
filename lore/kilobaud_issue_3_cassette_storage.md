# Kilobaud Issue #3: Cassette FSK Storage, Kansas City Standard, and Semafoon Selective Tone Decoding

This document provides a detailed technical analysis of the cassette tape storage formats presented in *Kilobaud Computing* Issue #3 (March 1977), drawing engineering parallels between tape FSK demodulators and the Dutch **Semafoon Selective Paging Decoder** in our synthesis studio.

---

## 1. Cassette Audio Storage: The Kansas City Standard (KCS)

Before low-cost floppy drives, microcomputer users stored data on standard audio cassettes. In the article *"The Gory Details of Cassette Storage"*, authors analyze the **Kansas City Standard (KCS)** (formalized in late 1975 at a symposium organized by Wayne Green).

### A. Modulation Mechanics
To bypass the limited high-frequency response of cheap tape recorders, KCS encodes digital bits as distinct audio frequencies:

*   **Logical 1 (Mark)**: represented by 8 cycles of a **$2400\text{Hz}$** sine wave.
*   **Logical 0 (Space)**: represented by 4 cycles of a **$1200\text{Hz}$** sine wave.
*   **Baud Rate**: $300\text{ bits per second}$.

### B. Hardware Demodulation
Reading KCS files requires a **phase-locked loop (PLL)** or a **comparator-based zero-crossing counter** to distinguish the $1200\text{Hz}$ and $2400\text{Hz}$ intervals, converting frequency shifts back to high/low logic levels for the UART.

---

## 2. Parallels to the Dutch Semafoon Decoder

In the **TSFi2 Synthesis Studio**, we run a virtual **Semafoon Decoder** module ([semafoonDecoder.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/semafoonDecoder.yul)), which simulates the 1964 Dutch selective calling pager network:

1. **Sequential Tone Decoding**:
   * The Semafoon system transmits pager addresses using sequential audio tones between $1130\text{Hz}$ and $2560\text{Hz}$ (almost identical to KCS bands).
2. **Frequency Shift Tracking**:
   * The Yul contract uses bandpass filters and comparator thresholds to track when these tones are present, decoding the selective call envelope.
3. **VM Processing**:
   * Both KCS and Semafoon illustrate the mapping of dynamic control data over standard audio-bandwidth mediums, allowing us to route control parameters via standard audio paths in our closed-loop studio.
