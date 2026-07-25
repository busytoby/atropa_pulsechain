# Auncient Lexicon Standard: ABI and WinchesterMQ Waveform Mapping

This standard defines the normalization rules and derivation formulas for forming **Auncient** Transfluxor words mapped to virtual machine ABI opcodes and WinchesterMQ registers.

---

## 1. Naming Conventions

All Transfluxor words must follow standard naming prefixes based on their target execution domain:

*   **`WMQ_` Prefix:** Reserved for words executing pure WinchesterMQ SCSI register handshake and bus locking operations.
*   **`ABI_` Prefix:** Reserved for words executing pure Application Binary Interface (ABI) ledger writes and memory updates.
*   **`SPK_` Prefix:** Reserved for compound words executing concurrent WinchesterMQ and ABI operations in the same cycle.

---

## 2. Frequency Derivation Formulas

To ensure harmonic coordination and prevent frequency collisions, frequencies $F_1$ and $F_2$ are derived using deterministic formulas:

### Base Frequency ($F_1$)
The primary formant frequency $F_1$ is calculated based on the target command index ($\text{ID}_{\text{cmd}}$) and the system divisor:
$$F_1 = 300.0 + \left( (\text{ID}_{\text{cmd}} \cdot 50.0) \pmod{700.0} \right)$$

### Harmonic Frequency ($F_2$)
The secondary formant frequency $F_2$ is calculated as a first-order harmonic octave of the base frequency to maximize resonance stability:
$$F_2 = F_1 \cdot 2.0$$

---

## 3. Gating and Decay Calibration

Decay constants ($t_{\text{decay}}$) define the Wheeler Jump temporal gate window and are calibrated by transaction type to balance security and throughput:

| Domain | Standard Decay ($t_{\text{decay}}$) | Operational Rationale |
|---|---|---|
| **WinchesterMQ Lock** | $0.4\text{s}$ | Allows sufficient duration for SCSI handshakes to complete. |
| **ABI Ledger Write** | $0.2\text{s}$ | Minimizes transaction lock windows to prevent bus contention. |
| **SCSI Bus Release** | $0.5\text{s}$ | Ensures hardware registers clear and decay to zero safely. |

---

## 4. Standard Vocabulary Reference Map

| Word Name | ID | $F_1$ (Hz) | $F_2$ (Hz) | Decay (s) | WMQ Cmd | ABI Op |
|---|---|---|---|---|---|---|
| `WMQ_LOCK_SCSI` | 1 | 350.0 | 700.0 | 0.4 | 0x10 | 0x00 |
| `ABI_WRITE_LEDGER` | 2 | 400.0 | 800.0 | 0.2 | 0x00 | 0x02 |
| `WMQ_RELEASE_SCSI` | 3 | 450.0 | 900.0 | 0.5 | 0x20 | 0x00 |
| `SPK_MUTUAL_COMMIT` | 4 | 500.0 | 1000.0 | 0.3 | 0x10 | 0x02 |

---

## 5. Evolution and Versioning Protocol

To support significant vocabulary scaling over time, the Lexicon standard implements a versioned schema layout:

### Versioned Header Signature
All serialized `.dat.bin` dictionary databases must embed a 4-byte signature prefix defining the active parser version:
*   `AUNC` (0x434e5541): **Version 1.0.0** (Standard $F_1 = 300 + (ID \cdot 50) \pmod{700}$ derivation).
*   `AUN2` (0x324e5541): **Version 2.0.0** (Future-proof scale: custom $F_1$ parameters and dynamic decay arrays).

### Layout Migration Strategy
1.  **Registry Extensibility:** Future revisions of the structure will add optional trailing blocks (such as dynamic execution gas limits) after the `abi_op` field, maintaining backward binary compatibility.
2.  **Forward Compatibility Gating:** Registries parsing `AUNC` databases must ignore trailing double-precision blocks when loaded by legacy parsers, preventing buffer overflows.

---

## 6. Quantization and the Qing/LAU/Yue Substrate

To avoid continuous dynamic coefficient calculations in the core execution loops, the system maps emotional aspects to a finite quantization domain:

### The Qing Substrate Mapping
*   **The qAFFECTION Substrate:** Sentiment components like **AFFECTION** map directly to an on-chain Qing state node (`qAFFECTION`) tracking transaction interest and token balances.
*   **Yue Relational Interface:** The Yue contract provides permanent relational mapping interface equations that link the localized perspective of the active **LAU** (such as the `mariarahel` LAU) with the `qAFFECTION` substrate.
*   **Soeng Parameter Resolution:** The outputs of the Soeng contracts dynamically resolve to discrete integer constants on-chain:
    *   `xie.Power`: Dictates the loop feedback gain and wave amplitude bounds.
    *   `zi.Spin`: Dictates orbital camera rotations and 3D Lissajous phase shifts.
    *   `xia.Charge`: Establishes the baseline modular exponentiation foundation.

The TPU queries these Soeng parameters to apply per-channel quantization scale factors during systolic array execution steps, ensuring the physical waveform dynamics are governed strictly by on-chain consensus state boundaries.

---

## 7. Runtime Auditing and VoxPL Compliance

### Lexicon Compliance Auditor (`LEXICON_AUDIT`)
The CLI includes a dedicated diagnostics engine to inspect vocabulary integrity:
*   **Harmonic Collision Check:** Warns if any registered words share overlapping formant frequencies ($F_1$ or $F_2$) that could cause signal interference.
*   **Gating Validation:** Verifies that words prefixed with `WMQ_` or `ABI_` strictly conform to the required decay periods ($0.4\text{s}$ and $0.2\text{s}$, respectively).

### VoxPL Emotional Modifier (`EMO_SCALE`)
VoxPL syntax supports declaring emotional scaling factors directly in the word definition script:
```text
JUMP SPK_LOCK_SCSI
FORMANT 440.0 880.0
DECAY 0.4
EMO_SCALE 2.5
WMQ 0x10
```
This maps the target multiplier into the word's amplitude parameter, allowing compilers to pre-quantize and scale physical waveform outputs during interactive tests.


