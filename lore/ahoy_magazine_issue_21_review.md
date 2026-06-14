# Ahoy! Magazine Issue 21 (September 1985) — Technical & Historical Review

Published in September 1985, **Issue 21 of *Ahoy!* Magazine** focused on sprite graphics design tools, disk controllers (specifically the new Commodore 1571 drive), and basic editing utilities.

---

## 1. Program Listings Index

The following programs were featured in the "Program Listings" section of Issue 21:

| Program Title | Target System | Purpose / Architecture |
| :--- | :--- | :--- |
| **Go-Lister!** | C64 | A coding utility by John K. Lunde. Adds automated scrolling page navigation hooks to BASIC listings. |
| **Superhero** | C64 | A sprite-based action game by James C. Hilty. |
| **Moxey's Porch** | C64 | An adventure mystery text game by Bob Blackmer. |
| **Fastnew!** | C64 / VIC-20 | A fast disk formatting system. |
| **File Lock** | C64 | A program encryption security lock. |
| **Fish Math** | C64 / VIC-20 | An educational arithmetic game. |

---

## 2. Technical Code Analysis

### **Go-Lister! (John K. Lunde)**
A system tool that overrides the standard Commodore BASIC `LIST` command.
* **Architecture**: Hijacks the main character-out vectors at address `$0326` (CHROUT). When a listing begins, it intercepts the output stream, buffers the text by page block bounds, and waits for a keyboard key sequence before printing the next page.
* **Control**: Allows step-by-step assembly tracking, showing code without it scrolling off-screen in milliseconds.
