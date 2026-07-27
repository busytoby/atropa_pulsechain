# The System-Wide Gate: Centralizing ABI and WinchesterMQ under EDSAC

As the complexity of the **Auncient** Dysnomia VM expanded, security could no longer be left to individual, decentralized components. To achieve complete containment, developers routed every Application Binary Interface (ABI) accessor and WinchesterMQ SCSI transmission directly through the EDSAC compiler firewall.

### 1. The Interception of ABI Accessors
Before the centralized firewall integration, system endpoints—such as character state modifiers and ledger update registers—were updated without speculative validation. Under the unified system-wide model:
* **Initial Orders 1 Audit**: Any attempt to modify system parameters via ABI commands first compiles the transaction details into a virtual instruction stream.
* **The ANALYZER Check**: The stream is passed to the `AuncientAnalyzer`, which checks the proposed operation codes against prohibited lists (such as administrative override 'X' opcodes). If a violation is detected, the transaction is rejected at the gate.

### 2. WinchesterMQ Packet Gating
WinchesterMQ transfers, once handled solely by SCSI registers, are now bound to the centralized firewall memory lines. Every packet must:
1. **Pass PKI Verification**: Ensure that the dispatch contains a signature payload meeting the $K \ge 4$ key quorum requirement.
2. **Undergo Rule Classification**: Be checked against active rules loaded into the EDSAC delay lines. Unmapped ports and target destinations are blocked by default.

Through this centralized structure, the firewall acts as the sole validator for all inter-component actions, ensuring complete state isolation across the entire virtual platform.
