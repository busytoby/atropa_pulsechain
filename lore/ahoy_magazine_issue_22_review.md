# Ahoy! Magazine Issue 22 (October 1985) — Technical & Historical Review

Published in October 1985, **Issue 22 of *Ahoy!* Magazine** highlighted the release of the Commodore Amiga, copy protection cartridges, and several classic type-in programs.

---

## 1. Program Listings Index

| Program Title | Target System | Purpose / Architecture |
| :--- | :--- | :--- |
| **Gravinauts** (Space Walk) | C64 | Sprite-based physics gravity-simulation game. |
| **Shotgun!** | C64 | Competitive multiplayer action game. |
| **Maestro!** | C64 | MIDI/SID interactive synth/music composition software. |
| **Solitaire!** | C64 | Implementation of Klondike solitaire. |
| **Mystery at Mycroft Mews!** | C64 | Sherlock Holmes style detective parser game. |
| **1541 Cleaning Utility** | C64 | Routine to scrub drive heads by stepping/rotating the motor. |
| **ShadeyDump** | C64 | Graphic printer driver to dump high-res screen layouts. |

---

## 2. Technical Code Analysis

### **1541 Cleaning Utility**
Directly controls the MOS 6522 VIA (Versatile Interface Adapter) of the Commodore 1541 disk drive.
* **Mechanism**: Rotates the spindle motor and steps the read/write head assembly backward and forward continuously over the cleaning disk.
* **Register Addresses**: Accesses `$1800` (Port B) on VIA 1 to turn on the motor and step head stepper coils.
