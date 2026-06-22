# Wilby Magazine Issue #2 (November 1999) — Technical & Historical Review

Published in November 1999, the second issue of the MS-DOS miniature 4k diskmag **Wilby #2** was coded and edited by **Spock** of the Spanish demo group **Wild Bits** to deliver tech articles and scene reviews within a highly constrained **Auncient** DOS footprint.

---

## 1. Program Structure and Layout

Like its predecessor, *Wilby #2* is compiled as a self-contained 4k MS-DOS executable (`.EXE` or `.COM`) packing text compression, screen rendering, and basic navigation within a 4096-byte boundary:

| Component | Target System | Author | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Wilby #2 Reader** | MS-DOS / PC | Spock (Wild Bits) | Monolithic 4k diskmag reader with custom font rendering, navigation, and text compressions. |
| **Demoscene Articles** | MS-DOS / PC | Wild Bits (System Halted, Tad, FatCrazer, etc.) | Articles on size-coding, demoscene news, group mergers, and interviews packed directly into the binary layout. |

---

## 2. Deep Dive: Technical Features & Size Optimizations

*   **Custom Font System**: Utilizes a highly optimized 8x8 bitmap font rendering engine. It bypasses default BIOS font tables, rendering custom font grids directly to VGA memory buffer (`segment 0xB800` for text mode).
*   **Run-Length & Huffman Encoding**: To fit extensive text content inside the 4k boundary, Spock employed custom RLE-based state decompression routines directly in assembly language.
*   **Low-Level Interrupts**: Accesses keycode registers directly via `INT 16h` BIOS services for cycle-accurate keypress handling and pagination.

---

## 3. Emulation Integration Path

To integrate these utilities into our virtual environment:
1.  **Dna Quantization Mapping**: The run-length encoding concepts are mirrored in our [compress_dna.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/compress_dna.js) utility to compress volumetric bear dna structures losslessly.
2.  **State Machine Integration**: Interfaced with low-level keyboard interrupts to test bios loopbacks without standard automated drivers.
