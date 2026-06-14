# Ahoy! Magazine Issue 25 (January 1986) — Technical & Historical Review

Published in January 1986, **Issue 25 of *Ahoy!* Magazine** kicked off the new year with hardware modifications, reviews of MIDI keyboard expansion interfaces, and several C64/VIC-20 utility programs.

This review details the technical architecture and features of the ready-to-enter programs in Issue 25.

---

## 1. Program Listings Index (Ready to Enter)

The following programs were featured in the "Program Listings" section starting on Page 49:

| Program Title | Target System | Author / Notes | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Martian Monsters** | C64 | Arcade Shooter | A defensive shooter game where the player defends a surface grid against incoming alien sprites. |
| **The Haunted Castle** | C64 | Action/Strategy | A grid navigation exploration game where player choices trigger coordinate offsets within a maze array. |
| **Infraraid** | C64 | Defensive Radar | A radar defense grids simulation utilizing target detection points on coordinate axes. |
| **Streamer Font** | C64 | Custom Font | An interactive 8x8 bitmap generator that allows custom character cells to be saved to RAM ($2000). |
| **Knockout!** | C64 | Block Breaker | A wall-breaker game tracking dynamic physics boundaries and ball trajectory offsets. |
| **Alarm Clock** | C64 | CIA Utility | A utility demonstrating direct programming of the 6526 CIA real-time clock (TOD registers at `$DC08`–`$DC0B`). |
| **Memory Check** | C64 | System Utility | A scanning diagnostic program verifying RAM parity integrity from `$0800` to `$FFFF`. |

---

## 2. Technical Features & Implementation Details

*   **CIA TOD Registers**: The *Alarm Clock* demonstrates programming of the Time of Day (TOD) clock inside the MOS Technology 6526 Complex Interface Adapter (CIA). It requires unlocking registers by reading the tenths-of-a-second register, setting hours/minutes, and locking it back to begin ticking.
*   **Parity Diagnostics**: *Memory Check* scans memory blocks to verify retention. In our terminal simulation, this registers parity alerts when finding bad memory cells.
*   **Sprite Offsets**: *Martian Monsters* leverages custom VIC-II character maps and coordinates.
