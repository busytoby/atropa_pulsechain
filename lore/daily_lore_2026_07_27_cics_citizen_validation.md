# CICS Instruction Caller Validation via SSA Citizen Registry Checks

To prevent anonymous or unverified logic execution inside the **Auncient** core, the system-wide gate has been extended to validate the source of all instructions targeting CICS.

### 1. Default-Reject Gating
Under the default-reject posture, EDSAC holds absolute authority over packet routing. Subsystems, including CICS, have no direct or default access to the instruction compiler delay lines. Every incoming instruction stream must prove its origin before it can be parsed or executed.

### 2. SSA Registration Check
During the CICS execution cycle (`auncient_sdk_cics_exec`), the system intercepts the caller's ID (`writer_id`):
* **Resolution Pathway**: The analyzer forwards the writer ID to the material-to-cloth bridge. The bridge attempts to resolve a corresponding character entity name (e.g. `"TeddyBearSkelCharacter"` or `"AuncientFederalWorkerCharacter"`).
* **SSA Verification**: The resolved name is passed to `auncient_bridge_entity_to_ssa` to derive its Social Security Number and issuance site through the `tsfi_mf_ssa_resolve_issuance_site` registry.
* **Instruction Gating**: If the SSN fails to resolve, maps to the default unallocated SSN (`000-00-0000`), or if the writer ID is unregistered, the instruction is rejected at the gate (`[CICS REJECT]`) and immediately aborted, ensuring zero unrecorded or unauthorized execution.
