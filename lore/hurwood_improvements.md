# Hurwood Code Generating Programs: Proposed Technical Improvements

This document outlines proposed architecture and features to transition our `HURWOOD` code generators from static templates to a dynamic, parametric, and interactive generation engine matching Bernhardt J. Hurwood's vision of microcomputer automation.

---

## 1. Parametric Code Generation (CLI Arguments)

Currently, the generators output fixed static Commodore BASIC blocks. We can introduce options for custom configuration directly via command-line arguments:

```bash
# Syntax: HURWOOD [SCHEMA] [PARAMS...]
HURWOOD SOUND --freq 440 --waveform triangle --adsr 15 240
HURWOOD SPRITE --x 150 --y 120 --color 7 --multi 1
HURWOOD MAZE --density 0.75 --bg 1 --fg 0
```

### Proposed Argument Parsers
*   **`SOUND` Parameters**: Allow user-specified frequencies (A-4, C-5, etc.) or numerical frequencies, wave types (Noise, Pulse, Sawtooth, Triangle), and ADSR registers.
*   **`SPRITE` Parameters**: Allow coordinate changes, multi-color flag toggles, and memory pointer shifts.
*   **`MAZE` Parameters**: Adjust random generator thresholds or screen fill densities.

---

## 2. Interactive Code Builder Wizard

Instead of raw CLI commands, typing `HURWOOD BUILD` could launch a full screen text-user-interface (TUI) wizard inside the Wayland shell terminal, similar to early C64 productivity utilities:

```
+--------------------------------------------------+
|           HURWOOD INTERACTIVE BASIC BUILDER      |
+--------------------------------------------------+
|  1. Select Category: [SOUND] (Sprite, Maze...)   |
|  2. Waveform:        Triangle [Sawtooth] Pulse   |
|  3. Frequency (Hz):  440                         |
|  4. Attack Decay:    10                          |
|  5. Sustain Release: 180                         |
+--------------------------------------------------+
| [F1] Generate Code   [F3] Preview  [Esc] Exit    |
+--------------------------------------------------+
```

### Preview Integration
*   **Sound Preview**: Uses our internal host audio plugin to synthesize the SID sound sweeps on the host system prior to generating the BASIC code.
*   **Sprite Preview**: Renders a pixel grid of the sprite in a preview window using the Wayland terminal renderer.

---

## 3. Direct RAM/Virtual Disk Injection

Presenting code listings on the screen requires the user to manually type them in. We can bridge the generated code directly with the virtual machine:

*   **Export to File**: Add a write parameter (e.g., `HURWOOD SOUND --export "mysound.bas"`) to write a standard Commodore 64 `.PRG` format file directly to the virtual workspace drive.
*   **RAM Staging**: Inject the raw tokenized BASIC bytes directly into the C64 RAM start address (`$0801`) and update the BASIC end pointers (`$2D`/`$2E`) so that typing `LIST` or `RUN` immediately executes the program inside the emulated terminal.

---

## 4. Expanded Multi-Register Schemas

We can expand the generator database to support advanced hardware routines:

### 4.1 `HURWOOD SCROLL` (VIC-II Hardware Scrolling)
Generates code targeting the VIC-II horizontal/vertical fine-scrolling register `$D016` (53270) and `$D011` (53265) to implement smooth scrolling backgrounds for games:
```basic
10 FOR I = 0 TO 7
20 POKE 53270, (PEEK(53270) AND 248) OR I : REM FINE SCROLL
30 FOR T = 1 TO 50: NEXT T
40 NEXT I
```

### 4.2 `HURWOOD DOUBLE` (Double Buffering Page-Flipping)
Generates custom character and screen memory setups to swap active screen pointers dynamically, eliminating flicker on graphics updates.
