# Auncient CP/M-Tomie Pure C PDF Reader & Multimodal Analysis Specification
## Specification Document: TSFI-AUNCIENT-PDF-STD-2026-08

---

### 1. Scope & Sovereign Compliance
This specification standardizes the clean-room, pure C PDF extraction and multimodal feature analysis engine running under CP/M-Tomie TPA base $0x0100$. It provides lossless parsing of text streams, vector paths, embedded raster imagery, and typographic font CMaps without external third-party dependencies or hot-path overhead.

---

### 2. Core Architectural Components

| Component | Formal Role | Implementation Location | Standard Compliance |
| :--- | :--- | :--- | :--- |
| **RFC 1951 / 1952 Inflate** | Pure C Flate/Zlib Stream Decompressor | `tsfi_pdf_reader.c` | RFC 1951, RFC 1952 |
| **Octal / Ligature Filter** | Typographic character mapper | `tsfi_pdf_reader.c` | Standard Latin-1 / UTF-8 |
| **Multimodal Content Parser**| Text (`Tj`/`TJ`), Vector (`m`/`l`/`c`/`re`), Raster (`/Image`) | `tsfi_pdf_reader.c` | PDF 1.7 Spec (ISO 32000-1) |
| **TPA 0x0100 Binary Witness**| 3-term recurrence checksum verification | `cpm_tomie_pdf_reviewer.c` | Rule 18 Parity Checksum |

---

### 3. Strict Prohibitions & Invariants
* **No Python or External Scripting Engines**: The parser executes exclusively in compiled pure C.
* **No Hot-Path Console Prints**: Hot-path stream decompressions must not invoke console logging (Rule 11 compliance).
* **68KB File Limit**: The entire reader implementation must remain strictly under 68,000 bytes (Rule 8 compliance).
* **Memory Bounds**: All buffers utilize bounded dynamic allocation with strict null-termination and memory safety.

