# Ahoy! Magazine Issue 26 (February 1986) — Technical & Historical Review

Published in February 1986, **Issue 26 of *Ahoy!* Magazine** featured articles on 1541 disk drive alignment, auto-exec cartridge building, windowing systems, and a comprehensive flight simulator survey, alongside its popular type-in program listings.

This review details the technical architecture and features of the ready-to-enter programs in Issue 26.

---

## 1. Program Listings Index (Ready to Enter)

The following programs were featured in the "Program Listings" section starting on Page 55:

| Program Title | Target System | Author / Notes | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Arena** | C64 | Arcade Combat | A tactical grid-combat game where players engage in arena-style action, dodging obstacles and defeating opponents. |
| **Head to Head** | C64 | Strategy Board | A two-player strategy game challenging players in grid alignment and territorial containment. |
| **Crabfight** | C64 | Action Duel | A fast-paced arcade game where players control crabs battling for supremacy on a sandy shoreline, utilizing custom sprite registers. |
| **Treasure Wheel** | C64 / VIC-20 | Game Show | A puzzle game themed around spinning a wheel to guess hidden word phrases and collect coins. |
| **Character Dump** | C64 | System Utility | A machine-language screen diagnostics tool that dumps raw character memory layout and custom font mappings to the screen or printer. |

---

## 2. Technical Features & Hardware Emulation

*   **1541 Disk Alignment**: The issue's feature article outlines mechanical alignment of the Commodore 1541 disk drive read/write head relative to track 0 and track 35. This alignment process is replicated inside our drive head emulator.
*   **Sprite Customization (`Crabfight`)**: Highlights how C64 sprite pointers at the end of the screen RAM block (typically `$07F8`–`$07FF`) are used to drive multi-colored crab sprites, dynamically updating their coordinates via the VIC-II registers at `$D000`–`$D00F`.
