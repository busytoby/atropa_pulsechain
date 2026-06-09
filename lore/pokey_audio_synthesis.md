# Atari POKEY Audio & LFSR Synthesis

The **Atari POKEY (Potentiometer and Keyboard)** chip is a versatile peripheral integrated circuit used in the Atari 8-bit computers and numerous classic arcade cabinets. This document details POKEY's audio subsystem, focusing on its frequency dividers and polynomial counter noise generation.

---

## 1. Register Layout

POKEY provides 4 independent audio channels. Each channel has an 8-bit Frequency Divider register and an 8-bit Audio Control register.

### 1.1 Audio Frequency Registers (`AUDF1` to `AUDF4`)
* **Addresses**: `$D200`, `$D202`, `$D204`, `$D206`
* **Function**: Specifies the division factor ($N$) for the input clock source. The output frequency is:
  $$F_{out} = \frac{F_{in}}{2(N + 1)}$$

### 1.2 Audio Control Registers (`AUDC1` to `AUDC4`)
* **Addresses**: `$D201`, `$D203`, `$D205`, `$D207`
* **Bit Format**:
  * **Bits 0–3**: Volume (0–15)
  * **Bit 4**: Volume Only (Force output high/low, bypassing frequency divider for direct DAC control)
  * **Bits 5–7**: Distortion / Noise generator select (Poly-counter select)

### 1.3 Audio Control Global Register (`AUDCTL`)
* **Address**: `$D208`
* **Function**: Configures global clock sources (1.79 MHz vs. 64 kHz vs. 15 kHz), joins channels for 16-bit frequency resolution (linking channel 1+2 or 3+4), and toggles high-pass filtering.

---

## 2. Polynomial Noise Generators (LFSRs)

POKEY produces its signature sound textures (pure square waves, engine hums, explosions, white noise) by passing the frequency-divided clock through Linear Feedback Shift Registers (LFSRs) of different lengths:

| Distortion Mode | LFSR Length | Characteristics / Sound Type |
| :---: | :---: | :--- |
| **0b000 (0x00)** | 5-bit + 17-bit | Buzzing, metallic noise |
| **0b010 (0x40)** | 5-bit | Pure square wave (no LFSR filtering) |
| **0b100 (0x80)** | 9-bit | White/Pink noise, explosion rumble |
| **0b110 (0xC0)** | 17-bit | Pure square wave (same as 0x40) |

---

## 3. Simulation Implementation

To demonstrate this system, we have created a simulator script: [test_pokey_audio.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_pokey_audio.js).

This script:
1. Simulates POKEY registers in memory.
2. Implements a 9-bit LFSR poly-counter generator.
3. Computes output frequencies based on clock rate selection and channel dividers.
4. Generates a textual representation of the resulting sound wave.
