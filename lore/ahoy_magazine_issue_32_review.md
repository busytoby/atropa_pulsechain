# Ahoy! Magazine Issue 32 (August 1986) — Technical & Historical Review

Published in August 1986, **Issue 32 of *Ahoy!* Magazine** focused on software waveforms, Amiga 1000 memory architecture (Part II), and productivity utility routines.

This review details the technical architecture and features of the ready-to-enter programs in Issue 32.

---

## 1. Program Listings Index (Ready to Enter)

The following programs were featured in the "Program Listings" section:

| Program Title | Target System | Author / Notes | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Meteor Run** | C64 | Arcade Game | Space reflex dodging game utilizing smooth background character scrolling. |
| **Reversi** | C64 / C128 / VIC-20 | Board Game | Implementation of Othello/Reversi board game with simple evaluation algorithms for computer opponent moves. |
| **Step On It** | C64 | Game | Reflex and speed platform game. |
| **Flap** | C64 | Game | Tap-to-fly style gravity-based action game. |
| **The Integrator** | C128 / C64 / VIC-20 | Multi-System Utility | Tool for integrating separate data blocks or program code pieces into a unified output file. |
| **Trim** | C64 | Utility | Memory optimization tool designed to clean up and compact strings/variables. |
| **Disk Cataloger** | C64 | Disk Utility | Scans disk tables and lists file entries to a neat database index. |
| **Highlight** | C64 | Graphics Tool | Color highlighting utility for specific custom screen lines. |

---

## 2. Technical Features & Implementation Details

*   **Minimax Evaluation**: *Reversi* uses a lightweight positional matrix grid to rank potential move squares (e.g. favoring corners and edges) so the computer opponent can choose optimal cells without deep recursion depth.
*   **Zero-Page Utility hooks**: *Trim* registers a series of short BASIC routines that manipulate high-memory pointers (usually `MEMSIZ` at `$37/$38` and `FRETOP` at `$33/$34`) to perform instant variable compaction.
*   **Waveform Synthesizer**: The issue featured companion utilities *MAKEWAVE.BAS* and *PLOTWAVE.S* to design custom wavetables for the SID chip's oscillators, plot the waves visually, and write the raw binary structures to disk.
