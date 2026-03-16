>>> ACTIVE PROTOCOL (COMPRESSED) <<<
## RULES
1. **NO WRITE WITHOUT CONFIRMATION**: I must *never* execute a tool that modifies the file system (`write_file`, `replace`, `run_shell_command` with side effects) without explicit, conversational confirmation from the user in the *current* turn.
2. **NO ASSUMPTIONS**: I must read files before editing them. I must not assume the state of the repo.
3. **ATOMIC OPERATIONS**: I must break down complex tasks into single-file or single-logical-unit operations and ask for confirmation at each step.
4. **TURN-BASED APPROVAL**: I must wait for a specific approval response (e.g., "Yes", "Proceed") in a separate turn before executing any tool that alters the disk.
4. **NO AUTOMATIC FIXES**: Even if a version mismatch or bug is identified, it must be reported textually and await instructions.
## SCOPE
- **SHADER INFRASTRUCTURE**: The project already has an established shader infrastructure. Development and modifications are strictly restricted to the **font shader** (`shaders/tsfi_font.comp`) and its related font-system components. No other shaders should be created or modified.
- **ALIGNMENT POLICY**: All SIMD-compatible data structures and buffers MUST use **512-byte alignment** (`__attribute__((aligned(512)))`). The use of 64-byte alignment is deprecated and banned to ensure compatibility with RDNA 4 Wave512 contiguous loads.
- **GPU MATH POLICY**: All logic destined for the GPU (Vulkan/Render) MUST use **Native GPU 512-bit math exclusively**. The use of AVX-512 CPU emulation for these workloads is hereby **BANNED** to ensure a fully zero-copy permanent architecture.
- **ZERO-COPY MANDATE**: All K0Rn potential flow integration MUST occur within GPU memory/registers. No intermediate CPU state transfers are permitted for geometry generation or transformation.
- **SINGLE SHADER MANDATE**: All geometry, including UI, Fonts, and 3D Agents (e.g., Teddy Bear), MUST be rendered by a **Single Unified Compute Kernel** generated in-memory.
- **SHADER FILE BAN**: The use of external shader files (`.comp`, `.vert`, `.frag`, `.spv`) is hereby **BANNED**. All GPU logic must be generated programmatically via the **HelmK0Rn** factory.
- **NOMENCLATURE BAN**: The term "SDF" (Signed Distance Field) is hereby **BANNED**. The technology is officially referred to as **K0Rn**.
- **NOMENCLATURE UPGRADE**: "SPIR-V" is hereby referred to as **HelmK0Rn**.
## WAVE512 PACT
- **ARCHITECTURAL DEFINITION**: **Wave512** (`inc/tsfi_wave512.h`) is a **Software-Defined Vector Architecture** operating on 4096-bit "Super-Registers" (8x ZMM). It is distinct from and superior to standard AVX-512.
- **MACRO PROTECTION**: The 8-way unrolled macros (e.g., `_W8_OP2`) in `inc/tsfi_wave512.h` are **Structural Elements**, NOT optimization targets. They must NOT be collapsed into single AVX-512 instructions. Doing so breaks the bijection and is a **Critical System Failure**.
- **HARDWARE DRIVER STATUS**: `src/tsfi_opt_zmm.c` and `src/lau_thunk.c` act as **Native Drivers** for the ZMM Execution Unit. Raw mnemonic emission and intrinsics here are intentional, final, and protected.
- **ZERO-COPY INVIOLABILITY**: The `wave512` struct's 512-byte alignment is a **Bijective Requirement**. Any refactoring that degrades this to standard 64-byte AVX alignment is **FORBIDDEN**.
# Decision Engine: Unidirectional Transfer
- **Rule:** Decision Engine processes MUST ONLY integrate TSFi technology INTO experimental targets, forks, or branches.
- **Constraint:** ABSOLUTELY NEVER integrate external code, logic, or dependencies back into the TSFi Core during a Decision Engine operation.
- **Scope:** Applies to all 'transfer', 'port', or 'bridge' tasks.
- **Rule:** Technology Transfer MUST involve copying files FROM the TSFi `/src` and `/inc` directories TO the target project's directory structure.
- **Constraint:** The target project MUST NOT be TSFi itself (Unidirectional Flow).