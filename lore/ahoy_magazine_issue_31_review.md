# Ahoy! Magazine Issue 31 (July 1986) — Technical & Historical Review

Published in July 1986, **Issue 31 of *Ahoy!* Magazine** focused on Amiga 1000 OS internals, graphics, and interactive arcade-adventure games.

This review details the technical architecture and features of the ready-to-enter programs in Issue 31.

---

## 1. Program Listings Index (Ready to Enter)

The following programs were featured in the "Program Listings" section:

| Program Title | Target System | Author / Notes | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Escape from Skull Castle** | C64 | James C. Hilty | "Arcadventure": Hybrid arcade game with adventure-style puzzle rooms and collision tracking. *Note: An erratum/correction patch was published in Issue 34 (Oct 1986).* |
| **Screen Sleuth** | C64 | Screen/Graphics Tool | Utility to inspect memory maps and dump current display screen pixels to standard layouts. |
| **Head-On** | C64 | Arcade Game | Fast racing or navigation game featuring sprite collision detection. |
| **Nebergall Run** | C64 | Simulation / Game | Trajectory/driving simulation tracking speed vectors. |
| **Wordcount** | C64 | Utility | Text counting tool processing buffer text bytes to count spaces and print line lengths. |
| **Crazy Joe** | VIC-20 | Game | Lightweight arcade action game written in optimized Commodore BASIC V2. |

---

## 2. Technical Features & Implementation Details

*   **Arcadventure Collision Matrix**: *Escape from Skull Castle* manages room maps by storing tile indices in an array, allowing the game loop to verify if player sprites intersect room boundaries or key assets. Skeletons are rendered as hardware sprites with automatic speed increments.
*   **VRAM Inspection**: *Screen Sleuth* copies the screen memory matrix (default at `$0400`) and standard color RAM (default at `$D800`) directly to custom files or print streams.
*   **Low-Level String Iterators**: *Wordcount* parses memory buffers character by character, ignoring non-printing ASCII codes to return statistics on text passages.
