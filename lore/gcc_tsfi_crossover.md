# GCC-TSFi Crossover Integration Specification

This document details the plan to cross over **Auncient** `tsfi2` C code directly into the GCC backend, enabling compile-time execution and deep optimization of virtual hardware targets.

---

## 1. CICS-ALU Compile-Time Simulator
Instead of traditional constant folding, the compiler backend is linked directly against `libauncient_sdk.a`:

* **GIMPLE Folding Integration**: When the GIMPLE optimizer encounters vector operations or modular math, it invokes `auncient_sdk_alu_execute` directly inside `cc1`.
* **Compile-Time State Resolution**: Evaluates register values (e.g. `Channel`, `Monopole`, and `Chin` parameters) at compile-time using the host processor's AVX-512 hardware, replacing runtime code blocks with pre-calculated constant values.

---

## 2. Linking TSFi Libraries into GCC
We modify `tools/gcc/gcc/Makefile.in` to link the compiler backend (`cc1`) with the custom `tsfi2` libraries:

```makefile
# Add tsfi2 libraries to compiler linking rules
BACKEND_LIBS += -L$(srcdir)/../../ -launcient_sdk -ltsfi2 -lm -lrt -lpthread
```

This makes the full suite of Auncient cryptographic, vector, and state verification systems available to GCC passes.

---

## 3. Optimizing Code Generator Pipeline
* **Register Allocation**: Uses internal `tsfi_math` routines to dynamically analyze register lifetimes during register allocation passes (`lra.cc`), maximizing the residency of variables in the `zmm30` register.
* **Handshake Parity**: Compiler verifies STANAG broadcast structures and connection handshake logic compile-time, failing the build if signature requirements are not met.
