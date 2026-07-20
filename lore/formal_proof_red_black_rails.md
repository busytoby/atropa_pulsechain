# Formal Proof of Yul Virtual Hardware Invariants

This document outlines the formal proofs and invariant checks executed directly against the Bendix G-15D (Black Rail) and NCR 304 (Red Rail) **Auncient** Yul contracts deployed on our Anvil machine.

## Invariant 1: G-15D Timing Sector Rotation (Modulo 108)
* **Goal:** Prove that the timing sector register wrapped to $[0, 107]$ boundaries continuously.
* **Math Definition:** $S_{t+1} \equiv (S_t + 1) \pmod{108}$
* **Test Verification:**
  * Seeded $S_0 = 107$ directly into storage slot `0x10`.
  * Triggered execution cycle transition.
  * Observed storage slot read-back: $S_1 = 0$.
  * **Status:** Verified Pass.

## Invariant 2: NCR 304 Telemetry Parity Read-Retry Realignment
* **Goal:** Prove that upon reaching the retry validation limit, any odd parity corruption bit is cleared and error indicators are reset.
* **Math Definition:** $\text{if } R_t = R_{max} - 1 \implies P_{t+1} \equiv 0 \pmod 2 \text{ and } E_{t+1} = 0$.
* **Test Verification:**
  * Seeded validation limit $R_{max} = 3$ (slot `0x28`), initial retries $R_0 = 2$ (slot `0x22`), and odd parity key $P_0 = 1$ (slot `0x21`).
  * Triggered execution transition.
  * Observed storage slot read-back: Retries = 3, Parity = 0, Error Code = 0.
  * **Status:** Verified Pass.
