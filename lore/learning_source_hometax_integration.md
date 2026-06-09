# Learning Source, Inc. - Hometax Compliance Integration Blueprint

This document details the historical context of **Learning Source, Inc.** and outlines the integration blueprint for emulating their flagship product, **Hometax**, as an on-chain Yul tax calculation and compliance coprocessor for the TSFi2 platform.

---

## 1. Historical Context: Learning Source, Inc. & Hometax

**Learning Source, Inc.**, active in California during the early 1980s, was an ambitious home software publisher and retail computer pioneer. Their defining software product was **Hometax**:
* **The CP/M C64 Software Lifeline**: Originally compiled for CP/M-compatible Commodore 64 architectures, Hometax was one of the first home computer programs designed to guide users through the complex IRS Form 1040 layout using an interactive question-and-answer dialogue.
* **hotline Support**: Learning Source, Inc. innovated by offering a toll-free 800-number support hotline and periodic calculation updates for a small subscription fee.
* **IRS Review**: The program received positive recognition from the accounting firm Price Waterhouse and a benign nod from the IRS, establishing a precedent for automated consumer tax compliance.

---

## 2. On-Chain Hometax Coprocessor Memory Map ($D6A0–$D6AF)

To mirror the consumer financial calculation logic of Hometax at the protocol level, we introduce a Yul-based **Hometax Compliance Engine** remapped to the memory range `$D6A0`–`$D6AF` (54944–54959):

| Register Address (Hex) | Register Address (Dec) | Access Mode | Description |
| :--- | :--- | :--- | :--- |
| `$D6A0`–`$D6A3` | `54944`–`54947` | Read / Write | **Gross Taxable Income Pointer**: 32-bit value representing total accumulated tokens/gwei. |
| `$D6A4` | `54948` | Read / Write | **Deductions Category Selector**: Selects standard deduction class (0x01: Single, 0x02: Joint, 0x03: Corporation). |
| `$D6A5` | `54949` | Write-Only | **Calculation Trigger Strobe**: Writing `1` initiates the on-chain tax calculation sequence. |
| `$D6A6`–`$D6A9` | `54950`–`54953` | Read-Only | **Net Taxable Liability**: 32-bit result register displaying calculated tax due. |
| `$D6AA` | `54954` | Write-Only | **Tax Payment Release Strobe**: Writing `1` transfers calculated liability to the system treasury. |

---

## 3. Dynamic On-Chain Tax Calculation Flow (Yul)

When the Calculation Trigger Strobe (`$D6A5`) is activated with a non-zero write, the Yul operating platform executes the following logic:

1. **Read Gross Income**: Loads the 32-bit value stored across `$D6A0`–`$D6A3`.
2. **Apply Deductions**: Loads the category selector from `$D6A4`. Deducts the standard allowance:
   * **Single (0x01)**: Deduct 1,000 OTRT.
   * **Joint (0x02)**: Deduct 2,000 OTRT.
   * **Corporation (0x03)**: Deduct 5,000 OTRT.
3. **Bracket Calculation**:
   * If Net Income $\le$ 10,000 OTRT: Tax Rate is 10%.
   * If Net Income $>$ 10,000 OTRT: Tax Rate is 15%.
4. **Output Generation**: Writes the resulting calculated tax value to the read-only registers `$D6A6`–`$D6A9`.
5. **Clear Trigger**: Resets `$D6A5` to `0`.

---

## 4. Bridge to COMTAX and Diyat Ledger Systems

The Hometax Coprocessor coordinates directly with the active TSFi2 systems:

* **COMTAX Synchronization**: Calculations made in the `$D6A0` range can be imported by the **COMTAX** tax compliance coprocessor (`$D590`–`$D5AF`) to auto-verify capital gains ledger entries across distinct ERC-20 tokens.
* **Platform Treasury Depositing**: Poking `$D6AA` with `1` initiates an ERC-20 `transferFrom` equivalent from the user's mapped balance to the platform treasury (`0x1111111111111111111111111111111111111111`), emitting the dynamic event log:
  ```solidity
  event HometaxSettled(address indexed taxpayer, uint256 grossIncome, uint256 taxPaid);
  ```
  Topic1: `keccak256("HometaxSettled(address,uint256,uint256)")`.
* **State Verification**: These events are parsed by Web3 dashboard consoles to show real-time audit receipts, ensuring that the emulated 8-bit application calculations are verifiable on the public ledger.
