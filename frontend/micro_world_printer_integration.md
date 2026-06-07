# Micro World Electronix: MW-302 Printer Interface & On-Chain Print Spooler

This document details the virtual hardware design for emulating the **Micro World Electronix MW-302 Universal Parallel Printer Interface** (a popular interface allowing Commodore computers to output Centronics parallel print commands). It details how printer output is logged on-chain, and how printing is regulated via the **Diyat Printing Tax**.

---

## 1. MW-302 Emulation Memory Map ($D620–$D621)

To allow virtual 6502 programs to output text to a simulated paper printer, we emulate the MW-302 interface over a memory-mapped interface zone:

* **Data Register (`$D620` / `54816`)**: Contains the 8-bit character data byte currently sent to the printer bus.
* **Control/Strobe Register (`$D621` / `54817`)**: 
  * Writing a non-zero value (typically `1`) triggers the Centronics strobe line.
  * Triggering the strobe processes the byte in `$D620` and appends it to the on-chain spooler buffer.
  * Clears automatically back to `0` when complete.

---

## 2. On-Chain Print Spooler & Event Logger

Every character strobed through the MW-302 is collected into a line buffer in EVM storage. When a carriage return (`0x0D` or `\n`) is parsed, the accumulated line is flushed and journaled on-chain via EVM logs:

```solidity
event PrintLine(address indexed user, string lineText);
```

### Yul Spooler Dispatcher (Case 54817)
```yul
case 54817 { // MW-302 Strobe Trigger ($D621)
    sstore(getUserSlot(54817), val)
    if val {
        let char := and(sload(getUserSlot(54820)), 0xFF) // Read character from $D620
        
        // Dynamic Diyat Printing Tax: 1 OTRT unit per character printed
        let printTax := 1
        let userBal := sload(getUserSlot(848))
        if lt(userBal, printTax) { revert(0, 0) } // Revert if cannot pay tax
        sstore(getUserSlot(848), sub(userBal, printTax))
        
        // Transfer tax to platform treasury
        mstore(0x300, 0x1111111111111111111111111111111111111111)
        mstore(0x320, 848)
        let treasurySlot := keccak256(0x300, 64)
        let treasuryBal := sload(treasurySlot)
        sstore(treasurySlot, add(treasuryBal, printTax))
        
        // Emit TaxPaid event for the printed character
        log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), printTax)
        
        // Store character to buffer
        let bufIdx := sload(getUserSlot(54818)) // Buffer length counter
        sstore(getUserSlot(add(54822, bufIdx)), char)
        let nextIdx := add(bufIdx, 1)
        sstore(getUserSlot(54818), nextIdx)
        
        // Check for Carriage Return (\n / 0x0D) to flush print line
        if or(eq(char, 0x0D), eq(char, 0x0A)) {
            // Allocate memory to construct string and emit event PrintLine
            let logOffset := 0x300
            mstore(logOffset, 32) // String offset
            mstore(add(logOffset, 32), bufIdx) // String length
            for { let i := 0 } lt(i, bufIdx) { i := add(i, 1) } {
                let c := and(sload(getUserSlot(add(54822, i))), 0xFF)
                mstore8(add(add(logOffset, 64), i), c)
            }
            
            // Emit PrintLine: log2(memOffset, size, topic1, topic2)
            // Topic1: keccak256("PrintLine(address,string)") = 0xe4b...
            log2(logOffset, add(64, bufIdx), 0xe4b6e5cb2838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b999, getContextUser())
            
            // Clear buffer index
            sstore(getUserSlot(54818), 0)
        }
        
        sstore(getUserSlot(54817), 0) // Clear strobe
    }
}
```

---

## 3. Security Analysis of On-Chain Printing

Exposing a printer queue directly to the block log introduces specific vulnerabilities that are mitigated by our security design:

### 1. Spam Prevention (Sybil Resistance)
Writing to the contract event logs consumes node state and indices. Without resource costs, a malicious program could loop endlessly writing random characters, inflating node disk storage requirements. Enforcing the **1 OTRT Diyat Printing Tax per character** ensures that large print operations must be backed by real utility.

### 2. Sandbox Isolation
The buffer length counter (`54818`) and the accumulated print slots (`54822` and onwards) are prefixed by `getUserSlot()`. This prevents User A's print buffer from clashing with or corrupting User B's pending print lines.

### 3. Log Spoofing Prevention
The `PrintLine` log uses the authenticated `getContextUser()` as an indexed topic, ensuring that callers cannot spoof printed log records on behalf of other accounts.

---

## 4. Minter Token Receipt Printing

The virtual MW-302 printer interface can be used by 6502 assembly applications to generate formatted cryptographic "validation receipts" documenting on-chain token assets minted on the platform. On our local EVM, **only the PKIMinter (`pkminter.sol`) is active and deployed**, representing the primary available minting technology.

### Supported Active Minter:
1. **PKI Minter (`pkminter.sol`)**:
   - Prints cryptographic complexity validation receipts for PKI Tokens (such as the standard `PKI Tester` token).
   - Extracts signing key verification sets (complexity weight, used nonces, and signature proofs).

### Printing Workflow:
* **Minter Query (6502 KERNAL code)**: The CPU invokes the `PKIMinter` contract via the EVM Contract Call Bridge (`$D560`), retrieving token parameters (Complexity, Creator address, and active PublicKeys array).
* **Format & Spool**: The CPU formats the cryptographic parameters into a clean, legible certificate layout (utilizing borders `+---+` and alignment tags).
* **Hardware Strobe**: The character bytes are sent through `$D620` and strobed via `$D621` under the 1 OTRT Diyat Printing Tax, outputting to the block log.

#### Example PKI Certificate Print Output:
```text
+------------------------------------+
|  MICRO WORLD ELECTRONIX RECEIPT  |
|  MINTER: PKI TOKEN CERTIFICATE     |
+------------------------------------+
| Token: PKI Tester (㊵)             |
| Complexity Rating: 40              |
| Creator: 0xf39F...2266             |
| Signers Active: 20 Keys Registered |
+------------------------------------+
```
