# TSFi ZMM VM & TSFi Technology Notes

## 1. Wave512 ISA (Instruction Set Architecture)
**Concept:** A 512-bit Wide Vector Architecture derived from AVX-512 but abstracted into a "Wave" metaphor.
**Math Principles:**
-   **SIMD Parallelism:** Operations apply to 16 floats (single precision) or 8 doubles/integers simultaneously. This exploits data parallelism.
-   **Lattice Math:** Used in `tsfi_scramble_wave512` (in `src/tsfi_opt_zmm.c`) for memory scrambling. This relies on high-dimensional vector mixing (Rotations, XORs) to create cryptographic diffusion/confusion at memory bandwidth speeds.
-   **Density Functions:** The "Neurology" aspect uses mass density accumulation (integrals over time/execution) to trigger state changes.

**Key Files:**
-   `inc/tsfi_wave512.h`: Defines the `wave512` struct and maps high-level macros to GCC vector extensions and inline assembly.
-   `src/tsfi_opt_zmm.c`: The "ZMM Dispatcher" that manages the 32x512-bit register file and executes kernels.
-   `src/tsfi_zmm_vm.c`: The assembly interpreter parsing text commands (`WLOAD`, `WADD`) into Wave512 intrinsics.

## 2. Lau Memory (Wired Memory System)
**Concept:** A custom allocator that "wires" memory objects into a global registry, enabling introspection, provenance tracking, and zero-copy passing.
**Math Principles:**
-   **Bijective Mapping:** Every allocated pointer maps uniquely to a `LauWiredHeader` via offset arithmetic (subtraction).
-   **Hilbert Space Filling Curves:** Used in `src/tsfi_hilbert.c` (implied context) to organize memory or tasks spatially to optimize locality (cache coherence).
-   **Alignment Constraints:** Strict 512-byte alignment (matching ZMM register width) ensures that loads/stores never cross cache line boundaries inefficiently and enable aligned AVX-512 instructions (`vmovaps`).

**Key Files:**
-   `inc/lau_memory.h`: Public API (`lau_malloc`, `lau_free`).
-   `src/lau_memory.c`: Implementation of the allocator, header management, and registry locking.
-   `src/lau_registry.c`: A thread-safe global linked list (or hash map) of all active "Wired" allocations.

## 3. Thunk Proxy (Zero-Copy Switching)
**Concept:** A mechanism to switch execution context without copying data structures. It changes the "view" of data by changing the base pointer (R15).
**Math Principles:**
-   **Relative Addressing:** All data access is calculated as $P_{effective} = R_{15} + \Delta$. Changing $R_{15}$ transforms the entire data manifold instantly.
-   **Continuation Passing:** The thunk acts as a closure, capturing the "Next State" (function pointer) and "Context" (R15 value).

**Key Files:**
-   `inc/lau_thunk.h`: API for generating thunks.
-   `src/lau_thunk.c`: JIT compilation of machine code trampolines that set registers and jump.

## 4. Neurology (Synaptic Feedback)
**Concept:** An adaptive execution model where the workload modifies its own execution parameters based on runtime metrics.
**Math Principles:**
-   **Feedback Loops:** $State_{t+1} = f(State_t, Feedback_t)$. The `ZmmSynapse` struct captures $Feedback_t$ (density, pressure).
-   **Threshold Logic:** Simple discrete math ($Density > Threshold \implies Upgrade$) drives the kernel swapping mechanism.

**Key Files:**
-   `inc/tsfi_opt_zmm.h`: Defines `ZmmSynapse`.
-   `src/tsfi_opt_zmm.c`: Implements the feedback check and kernel pointer swap.

## 5. TSFi ZMM VM (MCP Server)
**Concept:** A JSON-RPC bridge allowing external agents (like LLMs) to write and execute Wave512 assembly.
**Integration:** It sits on top of the stack, parsing strings and invoking the `tsfi_zmm_vm` engine, which uses `lau_memory` for state and `tsfi_wave512` for execution.
**Key Files:**
-   `src/tsfi_mcp_server.c`: The main loop handling JSON-RPC over Stdio.
-   `src/tsfi_zmm_vm.c`: The assembly logic.

## 6. ZMM Escaping Protocol
Integrating C source code generation via LLM tools requires strict adherence to an escaping protocol to survive multiple layers of interpretation (Tool -> File -> C Compiler -> JSON-RPC).

**Protocol:**
1.  **Hex Escapes for Control Chars:** Use `\x0A` for newline and `\x09` for tab in C source to avoid ambiguity with literal newlines in the file.
    -   Prompt `\\x0A` -> File `\x0A` -> C `\x0A` (0x0A char).
2.  **Hex Escapes for Structural Chars:** Use `\x22` for quotes and `\x5C` for backslashes when checking characters.
    -   `if (*src == '\\x5C')` avoids `\\` confusion.
3.  **Aggressive Quote Escaping:** String literals containing quotes must use `\\\"`.
    -   Prompt `\\\"` -> File `\"` -> C `"` (quote inside string).
    -   Example: `strstr(line, "\\\"method\\\": \\\"exit\\\"")`.
4.  **JSON Output:** To output a backslash-escaped character in JSON (e.g. `\n`), the C code must print `\\n`.
    -   C string: `"\\n"`.
    -   Prompt: `\\\\n` (if using standard escapes) or `\\x5Cn` (safer).

## 7. Neurology Integration
The TSFi ZMM VM server now fully supports the TSFi Neurology System via the following ASM commands:
-   `WDISPATCH`: Executes the `tsfi_dispatch_zmm_dynamic` loop, triggering synaptic feedback and kernel swapping.
-   `WSET_DENSITY <float>`: Manually sets the synaptic mass density.
-   `WGET_DENSITY <Wn>`: Broadcasts the current mass density into a vector register for inspection.

**Verification:**
-   Tests confirm that:
    1.  Density updates are persisted across dispatch calls.
    2.  Kernel swapping (Low -> High) occurs when density thresholds are breached.

## 8. Robust Escaping Strategy (Update)
The "Hex Escape" protocol proved brittle due to multi-layer interpretation. The final robust solution relies on **Runtime Character Generation**:
-   Instead of string literals like `"\n"`, use `char delim[2] = {10, 0};`.
-   Use `snprintf` to construct format strings dynamically: `snprintf(fmt, ..., "%%s %%[^%c]", 10);`.
-   This bypasses the `write_file` -> `C Compiler` -> `Execution` escaping chain entirely for control characters.