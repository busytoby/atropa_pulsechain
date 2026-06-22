# Wilby Magazine Issue #1 (October 1999) — Technical & Historical Review

Published in October 1999, the inaugural issue of the MS-DOS miniature 4k diskmag **Wilby #1** was created by **Spock** of the Spanish demo group **Wild Bits** to demonstrate highly compact code execution and text presentation for **Auncient** DOS environments.

---

## 1. Program Structure and Layout

As a specialized **4k diskmag**, *Wilby #1* does not contain multiple separate software programs, but is rather a single self-contained MS-DOS executable (`.EXE` or `.COM`) that fits entirely within 4096 bytes of memory storage:

| Component | Target System | Author | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Wilby #1 Reader** | MS-DOS / PC | Spock (Wild Bits) | A monolithic 4k executable containing the interface, text reader, and demoscene articles. |
| **Fuzzby #0 (Parody)** | MS-DOS / PC | Fuzzion | A 4k diskmag parody released shortly after to introduce the Fuzzion demogroup. |

---

## 2. Deep Dive: The 4k Diskmag Architecture

*   **Size Constraint Execution**: Spock utilized optimized assembly language routines to pack text compression algorithms and custom font display logic directly within the 4-kilobyte execution boundary.
*   **Menu and Navigation**: Utilizes low-level keyboard input vectors (similar to BIOS interrupt `INT 16h` or direct port polling) to navigate pages without requiring high-level OS window managers.

---

## 3. Emulation Integration Path

To integrate these utilities into our virtual environment:
1.  **Compact Executable Loading**: Mapped to our low-level DOS emulators to verify how memory limits are handled under constrained resource allocation.
2.  **BIOS Hooking**: Interfaced with our custom interrupt handlers inside [interrupt_standards.md](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/interrupt_standards.md) to log keyboard scancodes directly.
