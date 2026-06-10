# Dan Fylstra’s SWEETS for KIM-1 (Byte Magazine, Feb 1978) & TSFi2 Integration

This document details the mapping, theoretical alignment, and practical integration of retro-computing concepts from **Dan Fylstra's "SWEETS for KIM: A Low Calorie Text Editor"** (published in *BYTE* Magazine, February 1978, Vol. 3, No. 2) into the **TSFi2** (Bijective Helmholtz Architecture / ZMM VM interpreter).

---

## 1. Concept Mapping & Architecture Matrix

| SWEETS for KIM-1 Concept | Original Hardware Implementation | TSFi2 Target Subsystem | Implementation Translation |
| :--- | :--- | :--- | :--- |
| **Simple String Entry & Edit** | 1KB RAM constraint, direct line editing via serial TTY/keyboard link. | **ZMM VM Terminal Interface** | Low-overhead text-processing buffer and string conversion utilities mapped to zero-page memory slots. |
| **Zero-Page Pointers** | Zero-page register indices (`$00-$FF`) to store text buffer boundaries (Start/End pointers). | **Yul EVM Zero-Page Storage** | Register mapping on storage slots to track string/payload boundary allocations natively. |
| **TTY Keyboard Polling** | Hardware terminal polling loop via KIM-1 ROM keyboard monitor (`$1A11`). | **Virtual Keyboard / Character Stream** | Intercepting terminal characters and mapping serial keystrokes directly to interpreter memory inputs. |

---

## 2. Deep-Dive: SWEETS-Style Memory Constraints & Zero-Page Pointers

In 1978, the MOS 6502-based KIM-1 shipped with only **1KB of RAM** (1024 bytes). Fylstra's SWEETS editor was designed to occupy a minimal footprint, leaving maximum room for users to enter, edit, and test software. To achieve this, it relied on zero-page pointers to dynamically reference the boundaries of the text buffer:
*   `$1E` / `$1F`: Start of text buffer.
*   `$20` / `$21`: End of text buffer (cursor position).

In the **TSFi2 Yul EVM Interpreter**, we emulate these boundary checks when reading and writing string payloads from our namespacing database:
1.  **Compact Layout**: Instead of large arrays, we serialize text records into contiguous memory.
2.  **Boundary Pointers**: We maintain zero-page style pointers in our Yul interpreter's scratch registers to dynamically calculate layout offsets.

---

## 3. Terminal Integration & keystroke Buffering

We integrate a simulated SWEETS line editor command loop inside our [protecto.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/protecto.html) and [app.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/app.js) terminal shell interface:
*   Directly mapping character key codes into the VM input register.
*   Enabling low-overhead line inserts and deletion operations mimicking Fylstra's original SWEETS command tokens (`I` for insert, `D` for delete, `L` for list).

> [!NOTE]
> Integrating Dan Fylstra's SWEETS editor concepts ensures that our virtual terminal emulation remains highly optimized, utilizing low-level memory offsets and avoiding high-overhead Javascript mock buffers.

---

## 4. Dan Fylstra's BYTE Issue #1 "Write Your Own Assembler" Jump Table & Bill Gates' Correction

In September 1975 (BYTE Issue #1), Dan Fylstra published **"Write Your Own Assembler"**, providing an early guide to two-pass symbol-table assembly mapping. A key technical feature was routing execution index offsets using a 3-byte jump table (e.g. `JMP target1`, `JMP target2`...).

The lookup index offset required multiplying the index by 3: `offset = index * 3`.
Fylstra's original code had a bug where it only shifted the index left (`ASL` / multiply by 2), causing routing collisions. Programmers (including Bill Gates) identified the issue, pointing out that Gates' correction `(index * 2) + index` was required for 3-byte jump table alignments.

We implemented a dedicated verification test at [scripts/test_branch_table.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_branch_table.js):
1. **Gates' Correction Simulation**: Index 1 is multiplied by 3 using `ASL A` followed by `CLC; ADC $10`, routing correctly to Routine 1 (`0xB0`).
2. **Bugged Run Simulation**: Multiplying by 2 causes offset routing to collide, attempting to execute an invalid offset segment (`0x31` address byte as opcode) and reverting with a CPU exception as expected.

