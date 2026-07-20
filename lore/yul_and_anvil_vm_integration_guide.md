# Yul Virtual Hardware & Anvil VM Contract Integration Guide

This guide maps the low-level Yul virtual register maps and the Anvil VM contract bindings used to verify cooperative sync hosts over the coaxial Kermit network.

---

## 1. Yul Virtual Hardware Registers

The system emulates WinchesterMQ SCSI virtual hardware state registers using direct memory mapping. Below is the active SCSI handshake loop register configuration:

### WinchesterMQ Register Map
*   `status_reg` (`0x00`): SCSI handshake status control word.
    *   `0`: `Idle`
    *   `1`: `Request` (Req)
    *   `2`: `Acknowledge` (Ack)
    *   `3`: `Complete`
*   `keycode_reg` (`0x04`): Keycode payload register (e.g. `30` for key 'A', `32` for key 'D').
*   `data_reg` (`0x08`): Transformed data payload buffer.

### SCSI Handshake Loop (WinchesterMQ.yul)
```yul
object "WinchesterMQ" {
    code {
        // Constructor: Initialize state registers
        sstore(0x00, 0) // Set status to Idle
        sstore(0x04, 0) // Clear keycode word
    }
    object "runtime" {
        code {
            let status := sload(0x00)
            if eq(status, 1) { // Request state
                sstore(0x00, 2) // Transition to Ack
                let keycode := sload(0x04)
                sstore(0x08, mul(keycode, 2)) // Shift data register
                sstore(0x00, 3) // Transition to Complete
            }
        }
    }
}
```

---

## 2. Anvil VM Contract Bindings

The **Anvil VM** manages logical state variables, variable bindings, and subgoal tracking via the WAM Trail.

### Bytecode Commands
*   `OP_VERIFY_ABDUCTIVE` (`0x5A`): Verifies abductive probability limits. If the posterior probability fails the limit check, the VM triggers a WAM backtrack:
    $$P(H \mid E) = \frac{P(H) \times 0.8}{P(E)} < \text{Threshold}$$

### Sync Host Contract Calls
During cooperative Kermit transfers, the sync hosts execute the following contract bindings on separate Rails VM contexts:

```c
// 1. Host A (Initiator) registers ETH Gas Spent and initiates transfer
tsfi_anvil_vm_bind(&sender_vm, "balance/ETH", "1000");
tsfi_anvil_vm_bind(&sender_vm, "gas/spent/ETH", "21000");

// 2. Host B (Verifier) registers Custom Gas Token and writes verification success
tsfi_anvil_vm_bind(&receiver_vm, "balance/LAU_GAS", "5000");
tsfi_anvil_vm_bind(&receiver_vm, "gas/spent/LAU_GAS", "100000");
tsfi_anvil_vm_bind(&receiver_vm, "status/kermit_transfer", "SUCCESS");
```

---

## 3. Coaxial ZMM Telemetry Logs

State transitions are logged and verified using direct command dispatches:
```sql
-- Register Host A as Initiator on ZMM
CALL sync_registry register_host_initiator SyncHost_A

-- Register Host B as Verifier on ZMM
CALL sync_registry register_host_verifier SyncHost_B

-- Register completed kermit verification on ZMM
CALL sync_registry complete_transfer SyncHost_A SyncHost_B SUCCESS
```
