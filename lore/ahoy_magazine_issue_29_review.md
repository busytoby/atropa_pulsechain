# Ahoy! Magazine Issue 29 (May 1986) — Technical & Historical Review

Published in May 1986, **Issue 29 of *Ahoy!* Magazine** focused on machine-language music programming, Epyx strategy guides, and several ready-to-type games and printing utilities.

This review details the technical architecture and features of the ready-to-enter programs in Issue 29.

---

## 1. Program Listings Index (Ready to Enter)

The following programs were featured in the "Program Listings" section starting on Page 45:

| Program Title | Target System | Author / Notes | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Alchemist's Apprentice** | C64 / C128 / Plus4 | Cheryl Cox Peterson | Text Adventure: parses user text input and checks state maps to solve alchemical puzzles. |
| **Failsafe** | C64 | Buck Childress | Disk Utility: system protection / validation utility for disk recovery and data integrity. |
| **Bigprint** | C64 | Paul Bolognese | Printing Utility: converts standard character bytes into large banner patterns. |
| **Ski Folly** | C64 | Kevin Brown | Sports Action: downhill skiing game shifting landscape lines dynamically to represent slope scrolling. |
| **English Darts** | C64 | Michael E. Townsend | Action Game: darts simulation utilizing vector coordination and trigger timing. |
| **Star Search** | C64 | Astronomy Utility | Astronomy: plots stellar constellation coordinates on a low-res screen map. |
| **Free RAM Check** | C64 | Diagnostic | Diagnostics: scans memory maps and reports open space blocks. |

---

## 2. Technical Features & Implementation Details

*   **Adventure Parser**: *Alchemist's Apprentice* utilizes a custom two-word parser (verb-noun) algorithm, checking vocabulary lookup tables to route commands like `LOOK`, `TAKE FLASK`, or `MIX`.
*   **Disk Data Validation**: *Failsafe* operates on disk sector tables, tracking checksum values to ensure block reads and writes don't corrupt disk sectors.
*   **Volumetric Character Scaling**: *Bigprint* maps the ROM 8x8 character sets into expanded bit matrices to output 12x scaled patterns on standard dot-matrix printers.
