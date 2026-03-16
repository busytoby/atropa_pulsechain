# Strategy: ThunkSignature Meta-Thunks & Real-Time Sealing Telemetry

## 1. Zero-Cost Meta-Thunk Framework (Reflection)
Currently, `DEFINE_MAPPED_STRUCT` embeds `MAPPED_COMMON_FIELDS` (9 function pointers + 3 vars) into the start of every hardware-mapped structure. The `lau_wire_mapped_logic()` routine blindly casts the memory payload to `MappedCommon*` and writes specific thunk intercepts into those first 12 slots.

This creates a rigid topography. To achieve true bijective meta-wiring, we must separate the **geometry** from the **logic**.

**The Plan:**
- **Schema Generation:** Evolve `DEFINE_MAPPED_STRUCT` to accept a macro-generated reflection schema (a static array of `ThunkSignature` structs) defining exactly which members require which thunks.
- **Header Binding:** Store a pointer to this static schema inside the `LauWiredHeader`.
- **Dynamic Emission:** `lau_wire_mapped_logic()` will iterate through the attached schema, read the byte `offset`, generate the JIT thunk, and inject it directly into the mapped object's memory.
- **Zero-Cost:** Because the schema arrays are static `const` data at compile time, and `offsetof` calculates precise geometry without runtime overhead, this provides dynamic polymorphism with zero abstraction cost.

## 2. Real-Time Telemetry for Sealing Boundaries
We will inject new event types into the TSFi Glass Cockpit telemetry stream to visually report when physical bounds are clamped or released.

**The Plan:**
- **Event Types:** Define `LAU_TELEM_TYPE_SEAL = 4` and `LAU_TELEM_TYPE_UNSEAL = 5` inside `tsfi_types.h`.
- **Hook Injection:** Modify `lau_seal_object_loc` and `lau_unseal_object_loc` in `src/lau_memory.c` to explicitly invoke `lau_registry_report_event(ptr, size, TYPE, flags)` whenever the `(1ULL << 55)` protection bit transitions state.
- **Cockpit Visualization:** Update `src/tsfi_cockpit.c` to parse these new event types and increment dedicated counters or print a real-time event log for physical manifold bounding.