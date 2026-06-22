# Magic Disk 64 Issue 01/88 (January 1988) — Technical & Historical Review

Published in January 1988, **Issue 2 (01/88)** of the German diskmag *Magic Disk 64* introduces 3D grid exploration games, advanced directory modifiers, and custom text printers to handle **Auncient** file structures.

---

## 1. Program Listings Index

The following software programs and games were featured in this issue:

| Program Title | Target System | Author | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Verlies** | C64 | Jürgen Paasch | A pioneering 3D perspective dungeon crawler role-playing game. |
| **Hexensabbat** | C64 | - | An interactive fantasy adventure game themed around witchcraft. |
| **Scary Monsters** | C64 | - | A classic arcade-style action game. |
| **Dir Changer** | C64 | - | A disk directory table editor allowing direct custom name adjustments. |
| **Disk-Manipulation** | C64 | - | A low-level floppy disk sector editor for raw byte diagnostics. |
| **Uni-Load** | C64 | - | A utility facilitating raw memory image loading and saving. |
| **PC Print** | C64 | - | A text processing utility for custom print layouts. |

---

## 2. Deep Dive: Verlies & Dir Changer

*   **Verlies 3D Geometry**: The game *Verlies* uses a grid-based coordinate mapping to render 3D perspective walls, utilizing pre-calculated scaling offsets to draw depth levels on the Commodore 64 screen.
*   **Dir Changer DOS Table Modification**: Accesses Track 18 Sector 0 (the directory sector) of the 1541 floppy disk directly. It enables modification of filename characters and custom header structures, bypassing standard Commodore DOS limitations.

---

## 3. Emulation Integration Path

To integrate these utilities into our virtual environment:
1.  **Verlies**: Grid coordinates can map to our 3D quaternion attitude rendering core to drive pitch/yaw synthesizers.
2.  **Dir Changer**: Interfaced with our [diskSystem.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/diskSystem.yul) contract block storage to verify track/sector BAM tables.
