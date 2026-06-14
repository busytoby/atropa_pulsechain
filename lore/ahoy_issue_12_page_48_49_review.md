# Ahoy! Issue 12 (December 1984) — Pages 48–49 Review

On pages 48 and 49 of **Issue 12 (December 1984)**, *Ahoy!* magazine details the design and application of its signature **type-in proofreading utilities**, highlighting how hobbyists verified large BASIC programs.

---

## 1. Technical Context: The "Bug Repellent"

Type-in program listings in the 1980s were notorious for syntax errors introduced during manual keyboard entry. To mitigate this, *Ahoy!* utilized two specialized validation tools:

1. **The Bug Repellent**:
   * A small machine language utility loaded into memory before typing the target program.
   * Whenever a user completed typing a line of BASIC and pressed **RETURN**, the utility calculated a checksum of the line's bytes and printed a 3-character verification code on the screen.
   * Users compared this code with the printed values in the magazine's column margins. A mismatch immediately flagged a typing error.

2. **The Line Total System**:
   * Provided a static table at the end of program listings.
   * Summed the ascii values of each line's instructions.

---

## 2. Page 48–49 Operational Guide

The text on pages 48 and 49 breaks down the workflow for using these checksum proofreaders:

* **Relocation**: The Bug Repellent machine code was designed to reside in a safe memory location (typically starting at `$C000` / `49152` decimal) so that it would not be overwritten by the BASIC program being entered (which starts at `$0801` / `2049` decimal).
* **Control Characters**: The pages detail the bracketed notation used to represent Commodore graphics characters (e.g. `{SC}` for clear screen, `{BL}` for blue text color, or `{CR}` for carriage returns). Because standard printers could not easily print these proprietary Commodore PETSCII symbols, the bracketed labels served as translation guides.
