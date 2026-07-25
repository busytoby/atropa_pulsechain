# Auncient Agent SDK Standards & Guidelines

This document outlines the architectural standards and integration specifications for the **Auncient Agent SDK** layers in the PL/EXUS environment. The SDK bridges physical/acoustic properties with logical machine directives to support safe, validated execution loops.

```
       +-------------------------------------------------------+
       |                  AGENT PROGRAMMING SDK                |
       +-------------------------------------------------------+
       |                                                       |
       |  +------------------+           +------------------+  |
       |  |  ACOUSTIC INPUT  |  ------>  |   COMPILATION    |  |
       |  | (Formant/Decay)  |           | (Unique Word ID) |  |
       |  +------------------+           +--------+---------+  |
       |                                          |            |
       |                                          v            |
       |  +------------------+           +--------+---------+  |
       |  |  SYSTEM OUTCOME  |  <------  |   RESOLVER HASH  |  |
       |  |  (ABI/WMQ Gating)  |           | (Motzkin Prime)  |  |
       |  +------------------+           +------------------+  |
       |                                                       |
       +-------------------------------------------------------+
```

---

## 1. Core Structures & Types

The SDK unifies physical signal variables and logical command routing under a single structured standard:

### `auncient_transfluxor_word_t`
The fundamental data type representing a vocalized command block:
*   `name`: Alphanumeric label for debug diagnostics (max 32 characters).
*   `word_id`: Unique transfluxor identifier mapping. Permits identical formant waves to resolve to distinct logic registers.
*   `f1, f2`: Formant carrier frequencies mapping to RLC filter bands.
*   `decay`: Vactrol LDR release duration (execution window limits).
*   `amplitude`: Wave intensity. A Wheeler Jump requires a saturation spike of $\ge 4.5\text{V}$.
*   `wmq_cmd`: Associated WinchesterMQ SCSI channel command.
*   `abi_op`: Associated ABI Accessor instruction opcode.

### `auncient_transfluxor_registry_t`
The vocabulary lookup database structure. Tracks up to `MAX_REGISTRY_WORDS` ($32$) registered words.

---

## 2. API Reference

All agent deployments must communicate with the VM using these standard C library functions:

### Hashing
```c
uint64_t auncient_sdk_calculate_transfluxor_hash(double f1, double f2, double decay, uint32_t word_id);
```
Calculates a unique 64-bit lookup hash using the system's modular divisor field:
$$\text{Hash} = (\text{round}(F_1) \times 1000 + \text{round}(F_2) \times 10 + \text{round}(\text{decay} \times 100) + \text{word\_id}) \pmod{\text{MotzkinPrime}}$$

### Compilation
```c
bool auncient_sdk_compile_transfluxor_word(auncient_transfluxor_word_t *word, const char *name, uint32_t word_id, double f1, double f2, double decay, uint32_t wmq_cmd, uint32_t abi_op);
```
Configures word fields and sets the default Wheeler Jump trigger amplitude ($5.0\text{V}$).

### Registration
```c
bool auncient_sdk_register_transfluxor_word(auncient_transfluxor_registry_t *reg, const auncient_transfluxor_word_t *word);
```
Adds the word to the registry. Halts and returns false if a hash collision is detected.

### Dispatching
```c
bool auncient_sdk_dispatch_transfluxor_word(const auncient_transfluxor_registry_t *reg, const auncient_transfluxor_word_t *word, double *feedback_freq);
```
Validates the Wheeler Jump amplitude. On successful registry lookup, returns true and sets `feedback_freq` to the success tone ($523.25\text{Hz}$). On failure, returns false and sets it to the warning drone ($110.0\text{Hz}$).

---

## 3. Integration & Deployment Standards

To preserve system safety, all agent modules (including Antigravity and spawned subagents) must adhere to these three integration rules:

1.  **Coaxial Loopback Transmission:** Agents must write compiled words to the shared Unix socket loops (`socket_fds`). This simulates physical coaxial transmission and forces all commands through the hardware gating filters.
2.  **Shared Registry Memory:** The vocabulary database must reside in a centralized, shared registry. Agents cannot define ad-hoc registries; they must query the shared master registry to ensure coherent word mapping.
3.  **Continuous Hash Verification:** Pre-commit hooks and validation workflows must parse compiled `.dat.bin` files and verify their calculated hashes against the shared registry before finalizing state modifications.

---

## 4. Acoustic Passwords & Security Windows

Speaking an acoustic handshake functions as a dynamic **acoustic password** rather than a static authentication token:
*   **Dynamic Wave Authentication:** Instead of sending cleartext strings, the agent speaks a complementary wave profile. The password is only validated if the summation of the challenge and response frequencies achieves perfect resonance ($1000\text{Hz}$ / $2000\text{Hz}$).
*   **Temporal Expiration:** The Wheeler Jump opens the validation window, which naturally closes as the vactrol LDR decays. This ensures the acoustic password expires within milliseconds, preventing replay attacks.

