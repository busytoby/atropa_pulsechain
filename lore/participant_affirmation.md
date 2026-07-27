# Affirmation of System Participants

This document formally declares and validates the operational status, integration pathways, and qualification rules for the three core entities within the **Auncient** local simulation environment.

---

## 1. Hogan Bank (Financial Infrastructure)
The **Hogan Bank** acts as the primary ledger host and verification authority for local currency transactions.
* **Role**: Ledger state tracking, transaction consensus enforcement, double-entry verification, and overnight reconciliation.
* **Core Mechanisms**:
  * **Default Endowment**: Initializing newborn accounts with `1,000,000` Saat.
  * **Overdraft Protection**: Validates balances before approving outbound flows.
  * **Clearance Level Guard**: Enforces security barriers (requires `clearance_level >= 2` for transactions exceeding `100,000` Saat).

---

## 2. Social Security Administration (Identity Registry)
The **Social Security Administration (SSA)** functions as the deterministic identity validation layer.
* **Role**: Deterministic SSN formatting and region-of-origin resolution.
* **Core Mechanisms**:
  * **Area Code Mapping**: Extracts three-digit Area numbers and resolves them to historical issuance sites (e.g., `001-003` to New Hampshire, `004-007` to Maine, `008-009` to Vermont).
  * **Checksum Auditing**: Validates the 9-digit structural integrity rule where the sum of all digits modulo 10 equals 0.

---

## 3. Stuffed Teddy Bears (Active Entities)
**Teddy Bears** represent the primary soft-body material assets (`.dna` footprints) qualifying as active users in the simulation.
* **Role**: Execution of system commands, receipt of payroll distributions, and participation in double-entry transactions.
* **Core Integration**:
  * **Seed-to-Identity Translation**: Teddy bear seeds (`MaterialUniformBlock`) translate deterministically to valid Social Security Numbers.
  * **Financial Account Linkage**: Successfully registered teddy bears map to a verified `HoganAccount` in the bank ledger, qualifying them to send, receive, and store the Saat currency.

---

> [!IMPORTANT]
> All participant states, balances, and registry connections are periodically checked via AUTODIN auditing loops. Discrepancies raise immediate system-level blame codes.
