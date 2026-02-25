# Future Architectural Plan: The Stack Protector Firewall

## Motivation
Currently, TSFi2 relies on GCC's `-fstack-protector-strong` heuristic to automatically protect vulnerable functions (e.g., those with local arrays or buffers) while ignoring high-frequency, math-heavy vector functions (e.g., AVX-512 K0Rn/SVDAG loops).

If external compliance mandates ever force the project to use `-fstack-protector-all` globally, it would devastate the performance of the Dysnomia simulation's transcendent physics engine by injecting TLS stack canary checks into every micro-function, causing severe instruction bloat, register pressure, and L1 cache thrashing.

## The "Firewall" Strategy
To safely support `-fstack-protector-all` without compromising performance, we will implement a physical and macro-level "firewall" around proven, mathematically safe functions.

### 1. Physical Segregation
Extract all pure mathematical, state-invariant, and high-frequency `inline` vector intrinsics currently scattered across modules (`tsfi_vec_math.c`, `tsfi_math.c`, `tsfi_svdag.c`, K0Rn engines) and consolidate them into a dedicated boundary, such as `inc/tsfi_transcendent_core.h`.

### 2. The Firewall Directive
Instead of decorating hundreds of individual functions with `__attribute__((no_stack_protector))`, establish the firewall at the file level.

Wrap the isolated files in a unified compiler directive:
```c
#pragma GCC push_options
#pragma GCC optimize ("no-stack-protector")

// ... All high-frequency, provably safe micro-functions ...

#pragma GCC pop_options
```

### 3. Engineering Enforcement
This physically divides the codebase into "Provably Safe / High-Performance" and "Standard / Protected" zones. Engineers must consciously submit code into the firewall, ensuring that the core Dysnomia physics engine remains immune to cache thrashing regardless of global `Makefile` flags.
