# Yul "Device Access" & Memory-Mapped I/O

In traditional 8-bit microcomputer architectures, the CPU interacts with peripheral hardware devices (such as the VIC-II video controller, the SID audio chip, or the CIA I/O ports) using **Memory-Mapped I/O (MMIO)**. Specific memory addresses are physically wired to control registers instead of RAM cells. Writing to or reading from these addresses triggers physical hardware actions.

In an on-chain emulated environment (like our Yul-based smart contracts), we can map EVM-level operations—such as external contract calls (`staticcall`, `call`), storage reads, or logging events—directly to emulated MMIO registers. This treats the blockchain state and external contracts as physical peripheral devices.

This design specification details the architecture of Yul "device access" and provides a registry map and code loop implementation.

---

## 1. MMIO and EVM Device Mapping

```
  6502 Guest Code                      Yul Emulator                       EVM Blockchain
  
  LDA #$05        ┐
  STA $D500       ┴─> Intercept write to $D500 ──> staticcall(Oracle) ──> Fetch price data
  
  LDA #$2A        ┐
  STA $D510       ┴─> Intercept write to $D510 ──> call(ERC20) ─────────> Transfer OTRT tokens
```

### 1.1 The External Device Registry
We map specific virtual register ranges inside our emulated memory space to native EVM operations:

| Register Address | Physical Hardware Analogy | EVM Mapping / Action |
| :---: | :--- | :--- |
| **`$D500` (54528)** | **Sensor Input Port** | `staticcall` to an **Acoustic Oracle** contract to fetch environmental parameters. |
| **`$D510` (54544)** | **Payment Output Register** | `call` to an **OTRT ERC20 Token** contract to transfer fees. |
| **`$D520` (54560)** | **Entropy Generator** | `keccak256` hash creation using block metadata (`timestamp`, `number`). |
| **`$D530` (54576)** | **Network Interrupt Strobe**| Triggering an `IRQ` or emitting an EVM event log for off-chain services. |

---

## 2. The Yul MMIO Interceptor Loop

Below is a Yul function demonstrating how memory access is monitored. If a guest CPU writes to the MMIO device range (`$D500–$D5FF`), the emulator translates the access into a native EVM call instead of writing to guest RAM.

```yul
// Method 23: executeMmioDeviceAccess(registerAddress, value, externalContractAddress)
// Selector: 0x2af6d9bc
if eq(selector, 0x2af6d9bc) {
    let regAddr := calldataload(4)
    let val := calldataload(36)
    let targetContract := calldataload(68)

    let responseValue := 0

    switch regAddr
    case 54528 { // $D500: Sensor Input (Oracle read)
        // Set up calldata in memory for staticcall: selector for getSensorData() -> 0x7a30cf19
        mstore(0x00, 0x7a30cf1900000000000000000000000000000000000000000000000000000000)
        
        // Call external oracle contract
        let success := staticcall(gas(), targetContract, 0x00, 4, 0x20, 32)
        if success {
            responseValue := mload(0x20) // Retrieve returned oracle data
            // Cache back into emulated device read buffer
            sstore(54528, responseValue)
        }
    }
    case 54544 { // $D510: Payment Output Register
        // Call transfer(address,uint256) -> selector 0xa9059cbb
        // Inputs: address val (the system treasury) and amount
        let treasury := 0x9f8841da92788e02012c2b71239e040f7b2291e5
        mstore(0x00, 0xa9059cbb00000000000000000000000000000000000000000000000000000000)
        mstore(0x04, treasury)
        mstore(0x24, val) // amount to transfer
        
        let success := call(gas(), targetContract, 0, 0x00, 68, 0x00, 0)
        sstore(54544, success) // Write success flag to status register
    }
    case 54560 { // $D520: Entropy Generator
        // Calculate hash on-chain
        mstore(0x00, blockhash(sub(number(), 1)))
        mstore(0x20, timestamp())
        responseValue := keccak256(0x00, 64)
        sstore(54560, responseValue) // Store generated entropy in register
    }
    default {
        // Standard RAM write: write directly to user isolated storage
        let userSlot := keccak256(caller(), regAddr)
        sstore(userSlot, val)
    }

    mstore(0x00, responseValue)
    return(0x00, 32)
}
```

---

## 3. Benefits of Yul MMIO over standard EVM Calls

1. **Hardware-Fidelity Integration:** Allows developers to write guest software (e.g. 6502 compiler targets) that behaves as if it is interacting with actual physical chips, while executing complex blockchain operations in the background.
2. **Dynamic Fee Collection:** Excises appropriate OTRT or native gas fees inside the MMIO case block only when a specific device function is called.
3. **State Integrity:** Completely separates standard memory registers from external interactive registers, maintaining a strict boundaries around user state sandboxes.
