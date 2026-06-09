# Unified Architectural Unit Testing Report

This document reports the unit testing coverage for the simulated systems integrated during this session. The test suite verifies the mathematical behavior, register maps, and operational efficiency of the simulated pipelines.

---

## 1. Test Suite Coverage Map

| Test Target | Simulated Chip/System | Verification Focus | Success Metric |
| :--- | :--- | :--- | :--- |
| **Progressive RAG** | Cascade search | Candidate filtering and FLOP reduction | $>100\times$ operations reduction |
| **GTIA Collisions** | Atari GTIA | Player/Playfield overlaps & `HITCLR` reset | Bitmask flag validity |
| **POKEY Audio** | Atari POKEY | Audio frequency dividers & 9-bit LFSR step | Correct frequency math & non-zero noise |
| **ANTIC Scrolling** | Atari ANTIC | Sub-pixel fine registers and LMS pointer jumps | Perfect tile coordinate boundary mapping |
| **Sega VDP Scroll** | Genesis VDP | Parallax line scroll, VSRAM column, & HUD Window | Proper layer priority mapping |
| **Sega VDP1 Quads** | Saturn VDP1 | Distorted quadrilateral bilinear UV calculations | Pixel center coordinate resolution |
| **YM2612 Synthesis** | Yamaha YM2612 | 4-operator phase modulation routing algorithms | Non-zero FM outputs across algorithms |
| **Genesis Dual-CPU**| M68K / Z80 | `BUSREQ` and `BUSACK` state lock validations | Access blocked under reset conditions |

---

## 2. Test Runner Execution

We have implemented a unified Node.js unit test runner: [run_unit_tests.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/run_unit_tests.js).

This test suite uses Node's built-in `assert` library to validate all functional invariants.
To run the test suite, run:
```bash
node scripts/run_unit_tests.js
```

---

## 3. Terminal Exploratory Test Suite & Telemetry Synchronization

We have implemented and verified a Python-based automated exploratory test suite at [test_terminal_exploratory.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_terminal_exploratory.py).

### Verified Features:
- **Editor Mode Prompt Safety**: Resolved rendering leaks by preventing the `zmm-vm>` command prompt from outputting inside editor screens (e.g. Wordcraft, DNATypewriter, and EasyScript).
- **VRAM Telemetry CPU Sync**: Mapped the terminal emulator's active screen/telemetry status directly to the Yul CPU VM memory space starting at address `0xF000` (61440).
- **Memory State Parsing**: Equipped the test runner with regular expression matching to fully parse `PEEK` command output, enabling real-time verification of register states and telemetry strings written back into the CPU memory.

To run the terminal exploratory test suite:
```bash
python3 tests/test_terminal_exploratory.py
```
