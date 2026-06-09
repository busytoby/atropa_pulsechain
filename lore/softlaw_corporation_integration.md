# Softlaw Corporation Integration Blueprint

Softlaw Corporation (Minneapolis, MN) was a pioneering 1980s software publisher best known for the **VIP Library**—a unified suite of productivity applications sharing a consistent interface design, command structures, and iconic representations inspired by the Xerox Star and Apple Lisa.

This document outlines the architecture, features, and technical paths for integrating Softlaw's design paradigms and productivity concepts directly into the Yul-CPU Wayland terminal environment.

---

## 1. The VIP Library Catalog

The VIP (Very Influential Programs) Library was designed to bring high-end workstation concepts to home computers (including the Commodore 64 and TRS-80 Color Computer):

| Program | Core Functionality | Wayland Terminal Mapping / Integration |
| :--- | :--- | :--- |
| **VIP Terminal** | 80/106-column soft-rendered terminal | Integrated standard terminal width, software-based high-res font scaling |
| **VIP Calc** | Spreadsheet with mouse/keyboard control | Embedded VM utility `CALC` (currently mapped to basic expression parser) |
| **VIP Database** | Flat-file structured database manager | Yul VM state file indexer & memory segment table explorer |
| **VIP Writer / Speller** | Word processor with unified visual commands | Embedded buffer text editor for script/Yul code generation |
| **VIP Accountant / Tax** | Ledger tracking and taxation calculation | Core bookkeeping simulation under the Yul gas/tax rules |

---

## 2. Technical Feature Integration

### A. Soft-Rendered 80/106-Column Text Mode (VIP Terminal)
The standard Commodore 64 display was restricted to 40 columns of text. Softlaw achieved 80 to 106 columns by bypass-rendering text in high-resolution graphics mode using custom 4x8 or 3x8 bitmapped fonts.
* **Wayland Implementation**: In [tests/test_wayland_terminal_shell.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_terminal_shell.c), we can use the software rasterizer (`draw_debug_text`) to switch dynamically between:
  - **Standard 80-Column Mode** (using an 8x16 font layout)
  - **Compressed 106-Column Mode** (using a custom 5x8 or 6x12 font layout) to maximize screen real estate when debugging large VM registers (like `zmm` vector registers).

### B. Lisa/Xerox Star Icon & Command Interface
Softlaw applications unified their command sets around simple, visual menus and icons.
* **Wayland Implementation**: Integrate a top menu bar or status line in the Wayland terminal displaying quick-action glyphs/icons for VM state controls (e.g., Run, Pause, Reset, Memory Dump, and Export).

### C. Unified VM Expression Calc (VIP Calc)
* **Wayland Implementation**: Expand the Yul VM's `CALC` terminal utility into a interactive command-line grid spreadsheet, enabling users to write formulas referring to memory addresses (e.g., `=MEM[0x1000] + MEM[0x1004]`).

---

## 3. Recommended Implementation Steps

1. **Font Extension**: Add a high-density font representation to `tsfi_staging` supporting smaller glyph footprints for the 106-column VIP mode.
2. **Spreadsheet Subsystem**: Build a lightweight interactive sheet processor script running inside the Yul CPU environment to emulate `VIP Calc`.
3. **Database Register Inspector**: Create a database utility (`VIP DB`) to store and query compiled Yul contract metadata, function hashes, and call telemetry history.
