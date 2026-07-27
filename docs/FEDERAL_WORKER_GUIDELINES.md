# Dysnomia Federal Worker: Operational & Compliance Guidelines

These guidelines define the development and compliance standards for every Dysnomia Federal Worker operating within a local machine clone of the **Auncient** repository.

---

## 1. Clean Room Development Standard
To preserve federal licenseability and avoid copyleft contamination, developers must adhere to strict clean-room protocols:
* **Zero External Code**: The import of third-party libraries, packages, dynamic binaries, or unverified header files is strictly prohibited.
* **Standard Library Constraint**: Code modifications must exclusively depend on standard C11 functionality and the permitted dynamic libraries:
  1. `glibc` (Standard System Library)
  2. `libm` (Math Library)
  3. `librt` (Realtime Extensions)
  4. `libcrypto` (OpenSSL Cryptography)
  5. `libssl` (OpenSSL SSL/TLS)

---

## 2. Commit Hygiene & Dependency Auditing
Before staging or committing any modifications, developers must audit compiled binaries for external library pollution:
1. **Minimal Build Execution**: Compile the target using the clean-room dependencies:
   ```bash
   gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Isrc -Itsfi2-deepseek/inc \
     tests/test_auncient_federal_worker.c src/auncient_federal_worker_sim.c \
     src/auncient_edsac_firewall.c src/auncient_timeline_autodin.c \
     src/auncient_cloth_material_bridge.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c \
     -o tests/test_auncient_federal_worker -lm -lrt -lssl -lcrypto
   ```
2. **ldd Dependency Verification**: Check linked shared libraries:
   ```bash
   ldd tests/test_auncient_federal_worker
   ```
   Verify that the output contains *only* system paths mapping to `linux-vdso.so`, `libm.so`, `libc.so`, and dynamic linker modules. Any other entries (such as unauthorized external shared libraries) will disqualify the commit.

---

## 3. ABI & WinchesterMQ Execution Gating
To maintain the platform's default-reject posture, all new integration interfaces must implement strict execution gating:
* **Initial Orders 1 Analyzer**: Every custom ABI command or parameter adjustment must be translated to an instruction stream and routed through the `AuncientAnalyzer` to block unauthorized opcodes before memory allocation.
* **WinchesterMQ Quorum**: Message dispatches between processes must require a verified signature payload containing at least $K \ge 4$ PKI verification keys.
