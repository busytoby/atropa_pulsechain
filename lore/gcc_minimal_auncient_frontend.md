# Auncient Unified Front-End Integration for GCC

This document specifies the architecture to replace the standard GCC front-ends entirely with a minimal, unified compilation engine targeting **Auncient** systems.

---

## 1. Directory Structure (`tools/gcc/gcc/auncient/`)
We replace the multi-language front-ends with a single `auncient` compiler frontend directory containing:

* **`config-lang.in`**: Configures the front-end within the GCC build harness.
* **`auncient-lang.c`**: Implements GCC language hooks (type definitions, diagnostic initializers).
* **`auncient-parser.c`**: Unified lexer and parser that identifies file extensions:
  - `.algol61` (DDL structures)
  - `.cobol` (DML transaction blocks)
  - `.xplos` (low-level OS and kernel loops)
* **`auncient-gimplify.c`**: Directly translates the parsed syntax trees into target-neutral GIMPLE representations.

---

## 2. Minimized Compiler Configuration
To eliminate unnecessary components, the build system is configured to prune standard compiler suites:

```bash
./configure \
    --enable-languages=auncient \
    --disable-bootstrap \
    --disable-multilib \
    --disable-libsanitizer \
    --disable-libssp \
    --disable-libgomp \
    --disable-libquadmath \
    --without-headers
```

This configuration disables standard runtimes, producing a stripped, minimal `cc1`/`xgcc` toolchain dedicated entirely to `tsfi2` architectures.

---

## 3. Targeted Optimizations & ZMM Code Generation
* **AVX-512 Loop Alignment**: The code generator maps arrays directly to ZMM vector registers.
* **Residency Optimization**: Tracks signature quorums and hardware address constraints compile-time, emitting error warnings if signature requirements fail.
