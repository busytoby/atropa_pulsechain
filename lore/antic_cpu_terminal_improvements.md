# Improving the CPU Terminal with ANTIC Display Lists

The current Wayland CPU terminal shell ([test_wayland_terminal_shell.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_terminal_shell.c)) uses a monolithic, hardcoded rendering layout: a cyan divider line is drawn at a fixed coordinate ($y=45$), the header text is drawn at ($20, 15$), and characters are printed page-by-page via nested loops. Scrolling is implemented abruptly by shifting character row array indices.

By incorporating **ANTIC Display List** paradigms at the Wayland window level, we can fundamentally improve the terminal's architecture, adding support for **split-screen multi-pane layouts**, **smooth pixel-by-pixel scrolling**, and **zero-copy viewports**.

---

## 1. Architectural Enhancements

```
              Monolithic Rendering (Current)              Display List Rendering (Proposed)
              
              ┌────────────────────────────┐              ┌────────────────────────────┐
              │  Status Header (Hardcoded) │              │  Mode 2 Instruction (Bar)  │
              ├────────────────────────────┤              ├────────────────────────────┤
              │                            │              │  Mode 4 Instruction (Text) │
              │                            │              │  LMS: Point to Console RAM │
              │   Text Console (Loops)     │              │                            │
              │                            │              ├────────────────────────────┤
              │                            │              │  Mode 8 Instruction (Diag) │
              └────────────────────────────┘              └────────────────────────────┘
```

### 1.1 Dynamic Split-Screen Layouts (Multi-Mode Display Lists)
Instead of hardcoding status bar divider limits, the terminal layout is defined dynamically as an array of ANTIC instruction structures:
```c
typedef struct {
    uint8_t mode;             // Video rendering mode (Status bar text, Console text, 3D graphics)
    uint16_t screen_ram_ptr;  // Memory source address (LMS pointer)
    uint8_t line_height;      // Number of scanlines for this band
    uint8_t h_scroll;         // Fine scroll offset
} TerminalDisplayInstruction;
```
By simply altering the Display List instruction array, the terminal can instantly add a split-screen debug window or display real-time graphics charts at the bottom of the screen, with zero changes to the underlying drawing loop.

### 1.2 Zero-Copy Viewport Control (LMS Pointers)
Currently, scrolling the console requires iterating through characters and shifting row pointers in `g_vram->grid`.
* **Improvement:** The console text buffer is represented as a linear array. When the cursor moves past the visible screen limit, the terminal simply increments the `screen_ram_ptr` (LMS offset) of the active Display List instruction. This shifts the rendered console viewport instantly with **zero CPU cycles spent shifting characters**.

### 1.3 Smooth Sub-Character Scrolling (HSCROL/VSCROL)
* **Current Limitation:** Text scrolling is discrete; it jumps character-by-character (18 pixels vertically or 10 pixels horizontally).
* **Improvement:** Introduce `HSCROL` and `VSCROL` (0-7 pixel offsets). When the console receives input, the display engine increments the fine pixel scroll value. Once it reaches 8, it resets to 0 and increments the character offset pointer. This yields **smooth, modern pixel-by-pixel scrolling animations** for text consoles and files.

---

## 2. Display List Layout for the Terminal

Below is a proposed layout mapping the CPU terminal window:

| Scanline Range | Mode ID | LMS Memory Pointer | Description |
| :--- | :---: | :--- | :--- |
| **0–45** | **Mode 2 (Header)** | `$0400` (Status RAM) | Renders the static title, system status, and cyan divider line. |
| **46–650** | **Mode 4 (Console)**| `$0600` + Scroll Offset | Renders the interactive terminal character grid with fine vertical scrolling. |
| **651–720** | **Mode 8 (Gfx Graph)**| `$2000` (Diagnostics) | Renders raw diagnostic line graphs showing real-time JIT compile cycles. |

---

## 3. Verification Status

These architectural ANTIC paradigms are actively verified in our unified testing systems:
- **Homeword/WordPac Tests**: Emulate zero-copy viewports, word wrapping, and line-split pointers.
- **Automated Verification**: The automated exploratory suite [tests/test_terminal_exploratory.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_terminal_exploratory.py) executes these emulations inside the terminal dashboard and validates the telemetry output and CPU state changes.

