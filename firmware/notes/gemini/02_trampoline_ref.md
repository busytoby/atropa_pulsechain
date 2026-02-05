# Thunk / Trampoline Reference
**Date:** Wednesday, February 4, 2026

## Analysis of `firmware/notes/trampoline.c`
This file contains a proof-of-concept implementation of:
1.  **ThunkProxy**: Executable memory pool.
2.  **emit_baked**: Generation of thunks with fixed arguments.
3.  **emit_mixed**: Generation of thunks with mixed arguments.
4.  **WIRE_BIJECTION**: Macro to wire logic.

## Production Integration
This logic has been hardened and integrated into the `tsfi2` library component `lau_thunk.c` (available in `firmware/tsfi_lib/`).

**Key Improvements in `lau_thunk`:**
- **Lau Allocator Integration**: Uses `lau_malloc_wired` for lifecycle tracking.
- **Security**: Strict W^X enforcement (Write XOR Execute) via `mprotect` toggling.
- **Bijective Logic**: Integrated with `tsfi_wiring.c` for formal verification.

## Recommendation
Developers should use `lau_create_thunk_proxy()` and `lau_wire_bijection()` from `tsfi_lib` instead of copying code from `trampoline.c`.
