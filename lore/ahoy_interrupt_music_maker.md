# The Interrupt Music Maker/Editor (Ahoy! Issue 2, Page 39)

Pete Lobi's **Interrupt Music Maker/Editor** (IMM/E), published across Issues 1 and 2 of *Ahoy!* magazine in early 1984, represents a classic milestone in background music driver design for the Commodore 64. 

This document details the technical implementation, architectural design, memory layouts, and user command workflows of the software.

---

## 1. Architectural Design & Interrupt Model

Unlike typical early 8-bit tracker software that locked the CPU in busy loops during sound execution, the IMM/E utilized the C64’s **Raster Interrupt (IRQ)** to modulate the Sound Interface Device (SID 6581) in the background.

```
                      +----------------------------------+
                      |       Main Editor Loop           |
                      |   (Keyboard Inputs & UX Render)  |
                      +----------------------------------+
                                       |
                   [Hardware Interrupt - 60 Hz Raster Tick]
                                       v
                      +----------------------------------+
                      |    IRQ Vector ($0314/$0315 Jump)  |
                      +----------------------------------+
                                       |
                                       v
                      +----------------------------------+
                      |     Pete Lobi Playback Driver    |
                      |   - Modulates Freq low/high      |
                      |   - Applies Envelope Registers   |
                      |   - Toggles Gate/ADSR States     |
                      +----------------------------------+
                                       |
                                       v
                      +----------------------------------+
                      |        RTI (Return to Loop)      |
                      +----------------------------------+
```

### 1.1 Custom IRQ Hook
The C64’s hardware interrupt vector at memory addresses `$0314–$0315` was redirected to point to the editor's machine language playback routine. Every raster screen tick (approx. 60Hz NTSC), the playback code:
1. Checked if the playback gate was active (`*P` mode enabled).
2. Advanced the internal note pointer.
3. Updated the 3 voice channels dynamically.
4. Jumps back to the normal kernel IRQ routine at `$EA31` to handle the cursor blinking and keyboard scan matrix.

---

## 2. Command Set & User Interface

On **page 39** of the February 1984 issue, Pete Lobi details the editor's key entry syntax, focusing on how voice parameters and commands are parsed:

### 2.1 Playback Command: `(*P)`
*   Typing **`(*P)`** (with parentheses) triggers the **Play** subroutine.
*   Once triggered, the driver enables the IRQ gate, reading notes sequentially from memory blocks assigned to Voice 1, Voice 2, and Voice 3.
*   Typing any key during playback returns the editor to interactive command mode.

### 2.2 Voice-Specific Registries
Each note in the editor is stored as a character token. For example:
*   `C4` represents middle C.
*   `T/S/P/N` specifies the voice waveform:
    *   **T**: Triangle (Reg value: `$11` / `17`)
    *   **S**: Sawtooth (Reg value: `$21` / `33`)
    *   **P**: Pulse/Square (Reg value: `$41` / `65`)
    *   **N**: Noise (Reg value: `$81` / `129`)

---

## 3. Emulated SID Register Mapping (Voice 1 Focus)

To replicate the behavior of the IMM/E playback loop inside the on-chain Yul context, we map the C64 memory offsets to virtual storage registers:

| Hardware Address | Offset | Purpose | Yul Register Alias |
| :--- | :--- | :--- | :--- |
| `$D400` (`54272`) | `0` | Voice 1 Frequency Low | `V1_FREQ_LO` |
| `$D401` (`54273`) | `1` | Voice 1 Frequency High | `V1_FREQ_HI` |
| `$D402` (`54274`) | `2` | Voice 1 Pulse Width Low | `V1_PULSE_LO` |
| `$D403` (`54275`) | `3` | Voice 1 Pulse Width High | `V1_PULSE_HI` |
| `$D404` (`54276`) | `4` | Voice 1 Control Register | `V1_CTRL` |
| `$D405` (`54277`) | `5` | Voice 1 Attack / Decay | `V1_ATT_DEC` |
| `$D406` (`54278`) | `6` | Voice 1 Sustain / Release | `V1_SUT_REL` |

---

## 4. Key Fixes in Issue 2 (Part II)

In the second installment of the article series, Pete Lobi resolved two significant bugs reported from the first release:
1. **The Exit Crash Bug**: The initial ML driver forgot to restore the default hardware vector at `$0314` upon exit, leaving the CPU executing garbage memory when the user returned to BASIC. Part II added code to cleanly write `$31` and `$EA` back into the vector bytes before returning.
2. **Polyphonic Phase De-sync**: Voice 2 and Voice 3 envelopes sometimes fell out of sync due to clock division variances. The updated routine in Issue 2 synchronizes the gate-off ticks across all three channels.
