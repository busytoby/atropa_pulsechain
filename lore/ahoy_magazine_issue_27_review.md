# Ahoy! Magazine Issue 27 (March 1986) — Technical & Historical Review

Published in March 1986, **Issue 27 of *Ahoy!* Magazine** highlighted advanced game-maker utilities, telecommunications software, and machine-language enhancements for the Commodore 128.

This review details the technical architecture and features of the ready-to-enter programs in Issue 27.

---

## 1. Program Listings Index (Ready to Enter)

The following programs were featured in the "Program Listings" section starting on Page 51:

| Program Title | Target System | Author / Notes | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Ahoy! Term 128** | C128 | Telecom Terminal | An 80-column terminal software utilizing the C128's native MOS 8563 VDC controller chip. |
| **Trivia Game Maker** | C64 | Educational Tool | An interactive loader and database parser that reads custom trivia arrays and generates quiz viewports. |
| **Brickbusters** | C64 | Arcade Game | An action wall-breaking breakout clone using sprite collision and speed registers. |
| **Easy Lister** | C64 | System Utility | A text layout formatter that outputs program listings with clean indentation and page breaks. |
| **Programmer's Aid** | C64 | IDE Extension | A utility providing automatic line numbering, search/replace patterns, and basic instruction macros. |

---

## 2. Technical Features & Terminal Design

*   **VDC 80-Column Display**: *Ahoy! Term 128* utilizes the MOS 8563 Video Display Controller (VDC), bypassing the VIC-II completely to output high-resolution text at 640x200 pixels.
*   **VDC Registers**: Communicates via port `$D600` (Address Register) and `$D601` (Data Register) to write bytes directly to the VDC's private 16KB/64KB video RAM.
