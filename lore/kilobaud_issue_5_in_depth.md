# Kilobaud Issue #5: 1-Bit Software Sound Synthesis and Game Audio Lore ("Adding PLOP to Your System")

This document provides a detailed technical analysis of the game audio and software synthesis concepts presented in *Kilobaud Computing* Issue #5 (May 1977), focusing on 1-bit software-timed sound generators.

---

## 1. 1-Bit Sound Synthesis (Adding "PLOP" to Your System)

Before dedicated sound chips (like the AY-3-8910 or SN76489) became common in personal computers, hobbyists generated sound by connecting a single I/O port line (such as a pin on the MOS 6530 RIOT in a KIM-1) directly to a speaker through a simple transistor buffer.

### A. The "PLOP" Synthesis Loop
Since there was no digital-to-analog converter (DAC) or programmable sound generator (PSG), the CPU had to cycle the I/O pin high and low manually using software timing loops.
* **Square Wave Generation**: Alternating high/low writes with a fixed delay created a square wave.
* **Pitch Sweeps (The "PLOP")**: To create game sounds like explosions, coin drops, or ball bounces (the classic "plop"):
  1. The CPU starts by toggling the pin with a very short delay (high frequency).
  2. With each cycle of the wave, the software increments the delay counter, shifting the frequency lower.
  3. This produces a rapid, decaying pitch sweep ($1000\text{Hz} \to 100\text{Hz}$) over $\approx 150\text{ms}$.

### B. CPU Resource Constraints
A major disadvantage of this 1-bit software synthesis was that it was **fully blocking**. Because the timing was managed by CPU delay loops (e.g. nested 6502 `LDX/DEX` loops), the computer could not process game logic, read keyboard input, or update video displays while a sound was playing.

---

## 2. Application: Virtual PLOP Generator

In our **TSFi2 Synthesis Studio**, we implement a virtual **1-Bit PLOP Generator** that simulates this early software timing loop:
- Triggered by user interaction (`[ 🎮 PLOP ]`).
- Uses a Web Audio API oscillator modulated by a rapid, linear pitch-decay envelope to mimic the exact timbre of a software-timed 1-bit speaker output.
