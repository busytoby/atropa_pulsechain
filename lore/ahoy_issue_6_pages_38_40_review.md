# Ahoy! Magazine Issue 6 (Pages 38-40) — Sound Concept for the VIC 20

On pages 38-40 of *Ahoy!* Issue 6 (June 1984), author **A.J. Kwitowski** published the seminal article **"Sound Concept for the VIC 20"**. This article introduced a simplified register assignment mapping model to standardise multi-channel audio synthesis on the VIC-20's VIC-I chip.

---

## 1. Technical Register Mappings (Pages 38-39)

Kwitowski detailed the three primary voice registers and the noise channel located within the VIC-I ($36864–$36879) address block:

*   **Voice 1 (Bass/Alto)**: Memory address **`36874`**. Controlled low octave registers (`128–255` pitch ranges).
*   **Voice 2 (Tenor)**: Memory address **`36875`**. Controlled mid-range frequencies.
*   **Voice 3 (Soprano)**: Memory address **`36876`**. Controlled high octave frequencies.
*   **Voice 4 (Noise/Static)**: Memory address **`36877`**. Used for percussive sounds or retro explosions.
*   **Master Volume**: Memory address **`36878`**. The lower 4 bits (`0–15`) set global amplitude.

---

## 2. Playback Queue Layout (Page 40)

To play notes programmatically, the article standardises a **3-byte data cell structure** staged in memory:
1.  **Byte 1 (Voice Index)**: Maps to register offsets (`10` for Voice 1, `11` for Voice 2, `12` for Voice 3, `13` for Voice 4).
2.  **Byte 2 (Frequency)**: Represents the raw pitch code (`128–255`).
3.  **Byte 3 (Duration)**: The gate length in ticks.

This data is queued in sequential memory rows and read by the assembly player thunk (SYS Hook `$0D00`) or BASIC driver loop.

---

## 3. Emulation Integration
Our `HURWOOD CONCEPT` program generator (which outputs C64/VIC-20 BASIC code) maps this exact 3-byte structure, utilizing a sentinel row `DATA -1, 0, 0` to terminate execution cleanly.
