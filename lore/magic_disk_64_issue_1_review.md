# Magic Disk 64 Issue 12/87 (December 1987) — Technical & Historical Review

Published in December 1987, the inaugural issue of the German diskmag **Magic Disk 64** introduces automated loader routines, disk diagnostic tools, and custom sprite graphics animations to preserve the **Auncient** C64 software architecture.

---

## 1. Program Listings Index

The following programs were featured in the "Program Listings" section of the first floppy disk issue:

| Program Title | Target System | Author | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Deadly Rain** | C64 | - | An action game where the player shoots descending rain drops before they flood the screen. |
| **Runaway** | C64 | - | A fast-paced arcade game requiring high-speed navigation. |
| **Sprite Editor** | C64 | - | A utility for drawing, loading, and animating VIC-II sprites directly in RAM memory. |
| **Protector** | C64 | - | A security utility to encrypt and password-protect BASIC programs from cracking. |
| **Interrupt Module 1.0** | C64 | - | A machine-language utility adding command extensions utilizing system interrupt vectors. |

---

## 2. Deep Dive: Sprite Editor & Protector

*   **Sprite Editor**: Allows pixel editing of 24x21 monochromatic or multi-color VIC-II sprites, storing them in memory pages and facilitating real-time animations.
*   **Protector BASIC Security**: Scrambles BASIC pointers (e.g., `TXTTAB` at `$2B-$2C` or `VARTAB` at `$2D-$2E`), locking standard listing functions from cracker interception.

---

## 3. Emulation Integration Path

To integrate these utilities into our virtual environment:
1.  **Sprite Editor**: Mapped to our virtual [sally_larsen_c64.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/sally_larsen_c64.html) editor to import and preview custom sprite pages.
2.  **Protector**: Connected to our [protecto.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/protecto.html) system console to simulate bank-swapped BASIC program locks.
