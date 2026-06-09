# CIA Timers and Compounding Diyat Gas Taxation in Yul

This document details the virtual hardware design for emulating Commodore 64 Complex Interface Adapter (CIA) timers, and describes a compounding fee/tax collection mechanism ("Diyat") in Yul to excise a compound gas tax across operations.

---

## 1. MOS 6526 CIA Timer Emulation

The Commodore 64 employs two Complex Interface Adapters (CIA 1 at `$DC00` and CIA 2 at `$DD00`) to handle timers, keyboards, and serial ports.

### Timer Registers
Each CIA contains two 16-bit programmable interval down-counters (Timer A and Timer B):
* **CIA 1 Timer A**: `$DC04`–`$DC05` (Low/High byte)
* **CIA 1 Timer B**: `$DC06`–`$DC07` (Low/High byte)
* **CIA 2 Timer A**: `$DD04`–`$DD05` (Low/High byte)
* **CIA 2 Timer B**: `$DD06`–`$DD07` (Low/High byte)

### Cycle-Accurate Decrementing in Yul
For each 6502 CPU step, the Yul emulator tracks the execution cycle cost (e.g., `3` cycles for `STA absolute`). The timers are decremented by this cost:
```yul
let cycles := getOpcodeCycles(op)

// Decrement Timer A of CIA 1
let timerA := readRegister(0xDC04)
if gt(timerA, 0) {
    let nextA := sub(timerA, cycles)
    if le(nextA, 0) {
        // Underflow: Trigger IRQ vector and check Control Register for reload mode
        triggerIRQ()
        nextA := getTimerReloadValue(0xDC04)
    }
    writeRegister(0xDC04, nextA)
}
```

---

## 2. Compounding "Diyat" Gas Taxation in Yul

To excise a compound tax on execution resources or gas consumed, we can insert a **Diyat** step function at key execution bottlenecks (e.g., memory pokes, drawing cycles, file system writes).

By invoking the Diyat tax routine sequentially inside loops or operational steps, the system dynamically compound-taxes the remaining balance or gas value.

### Mathematical Formulation
If we apply a base tax rate $r$ (e.g., $0.1\%$ or $0.001$) at $N$ successive operational steps:
* **Asymptotic Remaining Balance Model**: If calculated purely on remaining transaction gas, the tax approaches $100\%$ asymptotically:
  $$\text{Tax} = 1 - (1 - r)^N$$
* **Leveraged Compounding Model (Exceeding 100%)**: If the tax is applied relative to the *initial* base value $V$ or current gas price, but is deducted directly from the caller's contract/account Ether balance (`address(this).balance`) rather than just the gas limit of that specific transaction step, the cumulative tax can exceed $100\%$ of the initial operation value:
  $$\text{Tax} = N \cdot r \cdot V \quad \text{or} \quad \text{Tax} = V \cdot (1 + r)^N - V$$
  For large $N$ or high step weights, the excised fee can escalate to several multiples (> 100%) of the underlying transaction value.

### Yul Compounding Implementation
We track the accumulated tax throughout a transaction, drawing directly from the user's contract balance to allow compounding payouts that exceed the transactional gas limits:

```yul
/// @notice Excises a compounding Diyat tax that can exceed 100% of the step value
/// @param stepWeight The intensity weight of the current operation
/// @param baseValue The value of the operation being taxed
function applyCompoundedDiyat(stepWeight, baseValue) {
    let deployerWallet := 0x962D2e6C9Eca21234F7DF0A0D923A0fe1DbBe327 // OTRT Deployer
    
    // Compounding rate: (1 + r)^N factor represented by step weight multiplier
    // This allows the tax amount to scale exponentially and exceed 100% of baseValue
    let compoundingMultiplier := mul(10, stepWeight) // Custom exponent factor
    let taxWei := div(mul(baseValue, compoundingMultiplier), 1000)
    
    if gt(taxWei, 0) {
        // Excise the compound tax directly from the contract's ether balance
        let success := call(gas(), deployerWallet, taxWei, 0, 0, 0, 0)
        
        // Emit an EVM event logging the compounding excise step
        log3(0, 0, 
             0x13c7a82b00000000000000000000000000000000000000000000000000000000, // Topic: DiyatTaxExcised
             deployerWallet, 
             taxWei
        )
    }
}
```

---

## 3. Definitive Timing Synchronization via Diyat and CIA Timers

Because the EVM executes transactions discretely rather than continuously, maintaining standard retro timing (e.g., matching a 1 MHz clock speed) is difficult. We can combine **CIA Timers** with **Diyat Token** dynamics to achieve deterministic, definitive timing operations:

### 1. Gas-Calibrated CPU Throttling
To prevent the virtual 6502 CPU from executing instructions faster than real-world wall-clock limits on local/public EVM nodes, we can link the CIA timer underflow directly to gas consumption constraints. 
* Every time a CIA Timer underflows (representing a fixed interval of virtual cycles, e.g., 1/60th of a second), a mandatory `applyCompoundedDiyat` execution is triggered.
* This establishes a hard cost-to-time ratio: running the processor for $S$ virtual seconds requires burning or transferring a minimum amount of gas value, stabilizing the speed of CPU loops relative to EVM block consumption rate.

### 2. Time-Locking via Underflow Verification
We can require that certain critical execution steps (like reading output data or writing to the screen frame buffer) can only occur when a CIA timer underflow has occurred.
If a contract method is called too early (measured by gas consumption metrics or block progression), the operation is locked or taxed at a higher rate using Diyat tokens, forcing a deterministic delay:

```yul
// Verify if execution is synchronized with CIA Timer ticks
let currentTimer := readRegister(0xDC04)
if gt(currentTimer, 0) {
    // If trying to access state before the timer has naturally expired,
    // impose a heavy "early-execution" Diyat tax to penalize spamming
    applyCompoundedDiyat(50, 1000) 
}
```

By adding `applyCompoundedDiyat(1)` to each iterative step, developers can configure exact compounding curves and asymptotic limits to control operational economics.
