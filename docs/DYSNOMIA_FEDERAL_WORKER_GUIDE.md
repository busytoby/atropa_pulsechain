# Dysnomia Federal Worker: Local Simulation Environment Guide

This document outlines the official instructions for a Dysnomia Federal Worker to clone, build, execute, and verify the multi-phase federal worker simulation loop in their local machine environment.

---

## 1. Prerequisites & Environment Setup
The simulation runs on any standard Linux environment. Ensure the following minimal tooling is installed:
* **Compiler**: `gcc` (support for C11 standard)
* **Standard Libraries**: The simulation is written in pure C with zero external framework dependencies, relying exclusively on the following four standard libraries:
  1. `glibc` (Standard C Library)
  2. `libm` (Math Library)
  3. `librt` (Realtime Extensions)
  4. `libcrypto` (OpenSSL cryptography library for FNV-1a hashing)

---

## 2. Cloning the Repository
Retrieve the active **Auncient** codebase using `git`:
```bash
git clone <repository-url>
cd atropa_pulsechain
```

---

## 3. The Federal Worker Lifecycle Phases
The local simulation walks a raw mathematical coordinate through 4 specific validation phases to establish its explicit authorization for work:

### Phase 0: Unauthorized Fourier Imposition
* **Description**: Raw coordinate inputs representing Fourier spectral peak points enter the pipeline.
* **Gate Check**: Any opcode mapping outside ASCII `'A'` to `'Z'` is classified as **noise**. Noise is blocked, not logged, and consecutive violations (>= 3) trigger a DEFCON 2 alarm.

### Phase 0.5: EDSAC Qualification
* **Description**: Valid opcodes are evaluated by the Initial Orders 1 analyzer against prohibited opcode masks (such as administrative `'X'` overrides).
* **Gate Check**: Qualified values transition the candidate to `PHASE_QUALIFIED`.

### Phase 1: The Audit
* **Description**: The qualified Fourier peak is converted to a DNA footprint, salted with `0xAA55BBDD`, and hashed via FNV-1a.
* **Gate Check**: Validates the static, unalterable DNA footprint, transitioning to `PHASE_AUDITED`.

### Phase 2: Confirmation
* **Description**: The material-to-cloth bridge maps the DNA seed to a deterministic regional Social Security Number (SSN) and SSA issuance site.
* **Gate Check**: Establishes citizen legitimacy, transitioning to `PHASE_CONFIRMED`.

### Phase 3: Selection
* **Description**: Registers the candidate as an active participant in Hogan Bank with a default endowment of `1,000,000` Saat.
* **Gate Check**: Dispatches the transaction over the WinchesterMQ SCSI channel, which blocks execution unless a signature quorum of at least $K \ge 4$ PKI verification keys is provided.

---

## 4. Running the Minimal Local Simulation Test
Because the codebase is self-contained with no external graphics or framework dependencies, the compilation requirements are minimal. To compile and execute the complete verification suite locally, run:
```bash
# Compile and run using only the 4 standard libraries
gcc -Wall -Wextra -Werror -std=c11 -O3 -Iinc -Isrc -Itsfi2-deepseek/inc \
  tests/test_auncient_federal_worker.c src/auncient_federal_worker_sim.c \
  src/auncient_edsac_firewall.c src/auncient_timeline_autodin.c \
  src/auncient_cloth_material_bridge.c tsfi2-deepseek/src/tsfi_cade_imf_nato.c \
  -o tests/test_auncient_federal_worker -lm -lrt -lssl -lcrypto

./tests/test_auncient_federal_worker
```

Upon execution, the terminal will log each phase transition, verifying the noise blocks, DEFCON alarm escalations, and successful worker registration.
