# Cimarron Insta-Series: Investment Combo — Technical Analysis

In 1984, **Micro-Sci** marketed the **Cimarron Insta-Series**, a collection of productivity software bundles for the Commodore 64. The most notable productivity-themed bundle was the **Investment Combo**, consisting of three integrated programs designed to cooperate via data exchange:

*   **Insta-Calc**: A cartridge-based spreadsheet utility.
*   **Insta-Graph**: A line and bar graphing package that loaded Insta-Calc files.
*   **Insta-Vestor**: A portfolio manager for tracking stock purchases, quantities, and price ticks.

This document reviews the technical architecture of the Investment Combo and schedules an on-chain portfolio valuation and Return on Investment (ROI) engine in Yul.

---

## 1. Integrated Database Architecture

The primary innovation of the Cimarron Investment Combo was its **file interoperability**. At a time when Commodore 64 applications were heavily siloed due to memory space constraints, Cimarron established a shared file format:
1. **Insta-Vestor Data Output**: Exported stock valuation arrays containing quantities and history matrices.
2. **Insta-Calc Parsing**: Loaded the exported arrays directly into spreadsheet columns.
3. **Insta-Graph Plotting**: Read spreadsheet ranges to generate high-resolution bar and line charts on the VIC-II chip.

---

## 2. On-Chain Portfolio Engine: Insta-Vestor

To replicate the core valuation and ROI tracking logic of **Insta-Vestor**, we implement a financial evaluation contract in Yul.

### 2.1 Asset Slot Layout (128-bit packed slot)
Each stock or asset transaction is stored in a packed memory block:
*   **Bits `[0..31]`**: Share Count (unsigned integer, supporting fractional shares with 4 decimals, i.e., factor of 10,000)
*   **Bits `[32..63]`**: Purchase Price (fixed-point 16.16)
*   **Bits `[64..95]`**: Current Price (fixed-point 16.16)
*   **Bits `[96..127]`**: Ticker symbol (4-byte ASCII code, e.g. "AAPL")

### 2.2 Yul Portfolio Evaluator
The following Yul function iterates through a list of transactions, calculates total cost, current portfolio value, and returns the net Return on Investment (ROI) percentage (scaled by 100 for 2 decimal places):

```yul
// Calculates total portfolio cost, current valuation, and ROI %
// assetPtr: Start memory address of asset array
// count: Number of assets/transactions
// Returns: (Total Cost in 16.16, Current Value in 16.16, ROI percentage scaled by 100)
function evaluatePortfolio(assetPtr, count) -> totalCost, currentValue, roiPct {
    totalCost := 0
    currentValue := 0
    roiPct := 0

    for { let i := 0 } lt(i, count) { i := add(i, 1) } {
        // Load 128-bit packed asset word from memory (offset = i * 16)
        let offset := mul(i, 16)
        let word0 := mload(add(assetPtr, offset))      // contains shares and buy price
        let word1 := mload(add(assetPtr, add(offset, 8))) // contains current price and ticker

        let shares := and(word0, 0xFFFFFFFF)
        let buyPrice := and(shr(32, word0), 0xFFFFFFFF)
        let currPrice := and(word1, 0xFFFFFFFF)

        // Cost = shares * buyPrice / 10000 (adjusting for shares decimal scaling)
        let costVal := div(mul(shares, buyPrice), 10000)
        totalCost := add(totalCost, costVal)

        // Current Value = shares * currPrice / 10000
        let currVal := div(mul(shares, currPrice), 10000)
        currentValue := add(currentValue, currVal)
    }

    // Calculate ROI = ((Current Value - Total Cost) / Total Cost) * 100
    if gt(totalCost, 0) {
        let netProfit := 0
        let positive := 1

        if gte(currentValue, totalCost) {
            netProfit := sub(currentValue, totalCost)
        }
        if lt(currentValue, totalCost) {
            netProfit := sub(totalCost, currentValue)
            positive := 0
        }

        // Multiply by 10000 for percentage with 2 decimal points: (netProfit * 10000 / totalCost)
        let rawPct := div(mul(netProfit, 10000), totalCost)
        
        if positive {
            roiPct := rawPct
        }
        if iszero(positive) {
            // Represent negative ROI as negative signed value
            roiPct := sub(0, rawPct)
        }
    }
}
```

---

## 3. Cimarron Combo Matrix

| Program | Media Format | Memory Footprint | Primary Register Target |
| :--- | :--- | :--- | :--- |
| **Insta-Calc** | 8 KiB Cartridge | \$8000–\$9FFF | RAM formulas, cell matrix grids |
| **Insta-Graph** | Floppy Disk | \$A000–\$CFFF | VIC-II High-Res mode (\$D011 bit 5) |
| **Insta-Vestor** | Floppy Disk | \$1000–\$7FFF | Transaction SEQ files, stock logs |

---

> [!NOTE]
> Cimarron's decision to distribute *Insta-Calc* on cartridge was a strategic memory layout choice. Cartridges do not consume the C64's precious 64 KiB RAM, leaving maximum memory available for large spreadsheet calculations.
