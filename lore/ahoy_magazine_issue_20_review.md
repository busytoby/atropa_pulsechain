# Ahoy! Magazine Issue 20 (August 1985) — Technical & Historical Review

Published in August 1985, **Issue 20 of *Ahoy!* Magazine** focused on sound design on the Commodore 64, early GUI systems, and the upcoming Commodore 128 architecture.

---

## 1. Program Listings Index

The following programs were featured in the "Program Listings" section of Issue 20:

| Program Title | Target System | Purpose / Architecture |
| :--- | :--- | :--- |
| **Sound-a-Rama** | C64 / VIC-20 | A menu-driven sound effects console by Bruce Bartlett. Features 15 preset sound effects, a digital drum machine, and a virtual tape recorder. |
| **Windows** | C64 | A utility to define, stack, and render overlay text boxes (windows) with ease inside BASIC programs. |
| **Formatter** | C64 | A document text-formatting processor program. |
| **Disintegrator** | C64 | An action arcade/puzzle game. |
| **Fidgits** | C64 | A logic action puzzle game. |
| **Selectachrome** | C64 | A graphics color picker and editor utility. |

---

## 2. Technical Code Analysis

### **Sound-a-Rama (Bruce Bartlett)**
This program is an excellent showcase of the MOS 6581 SID chip's capabilities.
* **Architecture**: The program uses a centralized loop mapping keyboard inputs (`A` through `O`) to specific envelope configurations.
* **SID Addressing**: Accesses registers starting at `$D400` (54272):
  * **Voice 1 Frequency**: `$D400–$D401`
  * **Pulse Width**: `$D402–$D403`
  * **Control Register**: `$D404` (selects Triangle, Sawtooth, Pulse, Noise, and Gate status)
  * **Attack/Decay (AD)**: `$D405`
  * **Sustain/Release (SR)**: `$D406`
* **Drum Machine Function**: Simulates quick envelope sweeps using Noise waveforms and short Decay rates to create kicks, snares, and hi-hats.
