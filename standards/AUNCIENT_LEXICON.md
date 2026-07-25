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
