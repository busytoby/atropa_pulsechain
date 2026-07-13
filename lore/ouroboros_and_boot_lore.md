# Auncient VM: Ouroboros Feedback, Boot Manager & Peer Registry Lore

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

## 5. Dynamic Context-Specific Gas Controller
To protect VM resources from starvation attacks, the ZMM VM implements a dynamic gas scheduler:
* **System Operations:** Run with infinite gas (`UINT64_MAX`), bypassing resource constraints.
* **Untrusted MCP Peers:** Initialized with a minimal limit (`5000` gas). Exceeding this limit immediately rate-limits the session, intercepts the execution thread, and flags the connection for administrator review.

## 6. Per-Cycle Register Auditor (Rule 12 Enforcer)
The hypervisor performs in-line register auditing at every CPU execution boundary to prevent invalid states or banned calculations:
* **Child-Langmuir Interception:** If a guest thread attempts to execute or calculate the banned space-charge power law formula ($J \propto V^{3/2}$), the auditor intercepts the register state.
* **Accumulator Redirection:** Instantly zeroes out the guest accumulator and redirects the computed values to the quarantined non-preferential accumulator state model.

## 7. Unified Peer Discovery Registry
Under the Ouroboros routing framework, a "peer" represents any addressable communication target:
* **MCP Hosts:** Network nodes (`peer_mcp://127.0.0.1:3000`).
* **JIT Thunks:** Native C thunk functions (`peer_thunk://ALSA_AUDIO`).
* **Registers:** Guest CPU registers (`peer_register://cpu6502/A`).
* **Port Discovery:** A thread-safe directory structure (`tsfi_peer_discovery.c`) allows Yul code and thunk executors to register and query target handlers dynamically at runtime, enabling automated loopback connections.
