# On-Chain CPU Upgrades: Banking & Jiffy Clock

We will implement bank-switching (6509 style) and an integrated cycle-accurate Jiffy timer directly into our Yul processor state.

---

## 1. Multi-Page Memory Banking (6509 Architecture)

To expand addressable memory beyond 64 KB, we map the banking registers to zero-page slots `0x0000` (bank select for index registers) and `0x0001` (bank select for accumulator / general instructions):

```
+------------------------------------------+
| Bank Select Registers (Zero Page)        |
| - 0x00: IND_BANK (Indirect index bank)   |
| - 0x01: ACC_BANK (Accumulator data bank) |
+------------------------------------------+
| Bank Switching Mapping                   |
| - Memory read/write routes data from the |
|   active bank selected in 0x00 or 0x01   |
|   instead of defaulting to Bank 0.       |
+------------------------------------------+
```

### Yul Storage Key Calculation
$$\text{StorageSlot} = \text{keccak256}(\text{userAddress} \parallel \text{bankId} \parallel \text{wordIndex})$$

---

## 2. Integrated Cycle-Accurate Jiffy Timer

We store a 64-bit cycle counter (`cyclesExecuted`) inside the user's CPU state registers slot.

*   Every instruction opcode has a standard cycle duration (e.g. `NOP` = 2 cycles, `LDA absolute` = 4 cycles).
*   During step execution, we increment this counter by the exact instruction cycles.
*   **On-Chain Jiffy Clock**: Games query this register at address `0xfffc` to execute timing loops (such as audio durations and graphics refreshes) without relying on block timestamps.
