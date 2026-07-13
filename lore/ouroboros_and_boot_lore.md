# Auncient VM: Ouroboros Feedback & Boot Manager Lore

This document records the architectural details and operational lore of the newly deployed **Auncient** subsystems.

## 1. Ouroboros Feedback Loop
The feedback loop represents a closed-circuit communication bridge between the guest register sets and host JIT thunks. By routing data port writes ($DF00) through a dynamic, multi-LUN array, signals propagate to AVX-512, ALSA, or Vulkan destinations, returning back to the guest accumulator before the next instruction cycle. This loop achieves a native execution speed of **3.60 ns / loop** in C, maintaining sub-microsecond latency.

## 2. JIT W^X Memory Enforcement
To secure the thunk space from write-injection attacks, the hypervisor enforces **W^X (Write XOR Execute)** boundaries. The JIT thunk pool page is toggled between write-only permission and execute-only permission:
1. **Creation/Emission:** The memory block is marked `PROT_READ | PROT_WRITE`.
2. **Locking:** Once instructions are written and the compiler cache is cleared, `mprotect` changes permissions to `PROT_READ | PROT_EXEC` before jump execution is allowed.

## 3. 2,816-bit PKI Validator Consensus
The **Auncient** signature verification engine uses an 11-key `secp256k1` validator pool to secure block commits.
* **Aggregate Key Space:** 2,816 bits of total private key entropy (11 keys x 256 bits).
* **Slashing Event:** If a validator attempts signature forgery (such as Key #5 in byzantine tests), the Yul consensus contract (`ConsensusPKI.yul`) immediately slashes their collateral in storage slot `35` by deducting 4.0 PLS.

## 4. Helmholtz Unshared Container Boot Manager
To execute headless tasks safely without host environment pollution, the hypervisor utilizes user-namespace virtualization:
* **Reset Vector Interception:** The boot manager traps warm-resets at `$FF5C`, preserving active VM registers.
* **Option 0:** Launches low-level hardware diagnostics (loopback, stake audits, Verlet FET discharge solver).
* **Option 1:** Mounts an isolated rootfs structure via `unshare` and boots `/sbin/init` (the Helmholtz daemon) to perform system and directory space audits.
