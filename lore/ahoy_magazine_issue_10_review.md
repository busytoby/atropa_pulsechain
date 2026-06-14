# Ahoy! Magazine Issue 10 (October 1984) — Technical & Historical Review

Published in October 1984, **Issue 10 of *Ahoy!* Magazine** focuses on C64 bit-mapped graphics layouts, joystick programming arrays, and display extensions.

---

## 1. Program Listings Index

The following programs were featured in the "Program Listings" section:

| Program Title | Target System | Author | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Lawn Job** | C64 | - | A game where players cut grass with a lawnmower, steering around trees and rocks. |
| **Emerald Elephant of Cipangu** | C64 | - | A textual interactive fantasy parser game. |
| **VIC 40 Column Operating System**| VIC-20 | - | A display utility rendering 40 text columns on a standard 22-column VIC-20 screen. |
| **BAM Read & Print** | C64 | - | A disk utility that reads Track 18 Sector 0 and prints a sector allocation map. |

---

## 2. Deep Dive: VIC 40 Column OS (Character Graphics Rendering)

The VIC-20 hardware has a default text grid of 22 columns by 23 rows. The **VIC 40 Column Operating System** bypasses the hardware character generator:
*   **Custom Font Definition**: Sets up a custom $4 \times 8$ pixel font inside RAM.
*   **Software Framebuffer**: Uses high-resolution graphics mode ($160 \times 192$ pixels), mapping text inputs dynamically to write bits into the bitmap memory buffer.
*   **VIC Registers**: Sets register `36869` to point to the custom RAM framebuffer.

---

## 3. Emulation Integration Path

To integrate these utilities into our emulator workspace:
1.  **BAM Read & Print**: Connect to the `diskSystem.yul` contract to parse Track 18 Sector 0 and output sector usage representations.
2.  **VIC 40 Column Operating System**: Bridge custom character fonts to `graphicsSystem.yul` VIC registers.
