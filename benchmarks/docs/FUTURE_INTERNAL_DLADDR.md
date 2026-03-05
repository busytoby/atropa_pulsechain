# Future Architectural Plan: Internal ELF Reflection (dladdr)

## Motivation
Currently, `tsfi_logic.c` uses the GNU extension `dladdr()` (from `-ldl` / `-D_GNU_SOURCE`) to resolve execution addresses (`__builtin_return_address(0)`) back to their static source locations via `addr2line`.

While acceptable during Phase 1/Alpha, this introduces a non-bijective dependency on the external GNU C Library's dynamic linker state. Crucially, `dladdr` is fundamentally incapable of resolving dynamically JIT-compiled K0Rn/Thunk wavefronts, leaving a blind spot in the `LauWireFirmware` provenance engine.

## The Bijective Reflection Strategy
To achieve absolute, freestanding execution awareness without `-D_GNU_SOURCE`, we must replace `dladdr` with an internal ELF parser.

### 1. The Mechanic
We will implement an internal function `tsfi_raw_dladdr(void *addr, TSFiDlInfo *info)` that operates directly on the loaded memory boundaries of the process.

1.  **Locate Base:** Parse `/proc/self/maps` or use the `AT_PHDR` auxv vector to locate the `Elf64_Ehdr` (ELF Header) of the running binary in memory.
2.  **Traverse Segments:** Walk the `Elf64_Phdr` (Program Headers) to locate the `PT_DYNAMIC` segment.
3.  **Hash Lookup:** Parse the `.dynsym` (Dynamic Symbol Table) and `.dynstr` (String Table). Using the `.gnu.hash` or `.hash` tables, we can perform a direct offset calculation to see which static symbol bounds the given `addr`.

### 2. The Bijective Advantage
By building our own reflection thunk:
*   We eliminate the final `-D_GNU_SOURCE` and `-ldl` dependency, rendering the C codebase 100% freestanding from GNU extensions.
*   We can extend our parser to "understand" our internal `ThunkProxy` memory pools. If an address falls outside the ELF boundaries but *inside* a `LauWireThread` JIT pool, the reflection engine can deterministically map it to its K0Rn mutation or source struct, eliminating the "jit_thunk" blind spot.
*   The distance between a physical execution pointer and its provenance identity becomes mathematically zero.

### 3. Implementation Plan
This will require a new source file (`src/tsfi_elf_reflect.c`) containing the standard `Elf64_Sym`, `Elf64_Ehdr`, and `Elf64_Phdr` struct definitions and the memory-walking logic. 
