# Sierra Homeword On-Chain Page Layout & Print Preview Coprocessor

This document details the virtual hardware, registers, and Yul contract interface design for the **Sierra Homeword Coprocessor** emulation on the TSFi2 platform. It brings classic word processing page-layout and miniature print preview rendering on-chain under a unified **Diyat Spooling Tax**.

---

## 1. Emulated Sierra Homeword Register Map ($D6E0–$D7EF)

The Homeword Coprocessor registers and text buffer reside at `$D6E0`–`$D7EF` (55008–55279):

| Register Address (Hex) | Register Address (Dec) | Access Mode | Description |
| :--- | :--- | :--- | :--- |
| `$D6E0` | `55008` | Read / Write | **Left Margin**: Standard layout column margin (0–80, default 10). |
| `$D6E1` | `55009` | Read / Write | **Right Margin**: Standard layout column margin (0–80, default 70). |
| `$D6E2` | `55010` | Read / Write | **Justification Mode**: Text alignment (0 = Left, 1 = Right, 2 = Center). |
| `$D6E3` | `55011` | Read / Write | **Line Spacing**: Layout row spacing (1 = Single, 2 = Double). |
| `$D6E4` | `55012` | Read / Write | **Text Length**: Count of characters in text buffer (1–256). |
| `$D6E5` | `55013` | Write-Only | **Render Trigger Strobe**: Writing `1` initiates the layout & print preview calculation. |
| `$D6E6` | `55014` | Read-Only | **Status Register**: Execution result (0 = Idle, 1 = Success, 2 = Overrun). |
| `$D6F0`–`$D7EF` | `55024`–`55279` | Read / Write | **Text Buffer**: 256 bytes containing the text characters to format. |

---

## 2. On-Chain Print Preview Renderer (Yul)

When the Render Trigger Strobe (`$D6E5`) is poked with `1`, the Yul operating platform performs the following:

1. **Clear Screen RAM**: Resets all C64 Screen RAM characters (addresses 1024 to 2023) to empty space (`32`).
2. **Draw Preview Page Outline**:
   * Draws a border representing a page on the screen.
   * Page coordinates: columns 10 to 30, rows 2 to 22.
   * Border character used: `160` (solid block) or similar graphic characters.
3. **Format & Spool Text Preview**:
   * Reads margins and justification.
   * Translates the 0–80 margins into screen-relative coordinates within the page outline.
   * Distributes the text characters into layout rows.
   * Renders the layout blocks using character codes (e.g., solid blocks or hyphens) into Screen RAM.
4. **Deduct Spooling Tax**:
   * Deducts 2 OTRT units from the caller's balance, depositing them to the treasury.
5. **Emit Event Log**:
   ```solidity
   event HomewordRendered(address indexed user, uint256 characterCount);
   ```
   Topic: `0x50fdb39d7be5a0438cf1dfba392019ff88d8b634882dfba51912bc5527e0291f`.

---

## 3. Register Definition in cpu6502.yul

```yul
case 55013 { // Homeword Render Trigger ($D6E5)
    sstore(getUserSlot(55013), val)
    if val {
        // 1. Clear Screen RAM (1024 to 2023)
        for { let i := 0 } lt(i, 1000) { i := add(i, 1) } {
            sstore(getUserSlot(add(1024, i)), 32)
        }
        
        // 2. Draw Page Border
        // Top & Bottom Border (rows 2 and 22, columns 10 to 30)
        for { let col := 10 } lt(col, 31) { col := add(col, 1) } {
            sstore(getUserSlot(add(1024, add(mul(2, 40), col))), 160)
            sstore(getUserSlot(add(1024, add(mul(22, 40), col))), 160)
        }
        // Left & Right Border (rows 3 to 21, columns 10 and 30)
        for { let row := 3 } lt(row, 22) { row := add(row, 1) } {
            sstore(getUserSlot(add(1024, add(mul(row, 40), 10))), 160)
            sstore(getUserSlot(add(1024, add(mul(row, 40), 30))), 160)
        }

        // 3. Format Spooled Text
        let left := sload(getUserSlot(55008))
        let right := sload(getUserSlot(55009))
        let just := sload(getUserSlot(55010))
        let spacing := sload(getUserSlot(55011))
        let len := sload(getUserSlot(55012))
        
        if iszero(spacing) { spacing := 1 }
        if iszero(right) { right := 70 }
        
        // Scale margins to 18 columns wide page boundary (columns 11 to 28)
        let pageLeft := div(mul(left, 18), 80)
        let pageRight := div(mul(right, 18), 80)
        if gt(pageLeft, 17) { pageLeft := 0 }
        if gt(pageRight, 18) { pageRight := 18 }
        if lt(pageRight, add(pageLeft, 2)) { pageRight := add(pageLeft, 2) }
        
        let lineWidth := sub(pageRight, pageLeft)
        let startRow := 3
        let currentRow := startRow
        let currentCol := pageLeft
        
        let textIdx := 0
        for { } lt(textIdx, len) { } {
            if gt(currentRow, 21) { break }
            
            // Read character from buffer
            let char := and(sload(getUserSlot(add(55024, textIdx))), 0xFF)
            textIdx := add(textIdx, 1)
            
            // Convert to PETSCII character line representation
            // We represent letters as horizontal dashes (45) or blocks to show preview layout
            let previewChar := 45 // Hyphen
            if eq(char, 32) { previewChar := 32 } // Space remains space
            
            // Justification offset adjustment per line could go here
            let colOffset := add(11, pageLeft)
            sstore(getUserSlot(add(1024, add(mul(currentRow, 40), add(colOffset, currentCol)))), previewChar)
            
            currentCol := add(currentCol, 1)
            if eq(currentCol, lineWidth) {
                currentCol := 0
                currentRow := add(currentRow, spacing)
            }
        }

        // 4. Charge 2 OTRT Spooling Tax
        let tax := 2
        if exciseOnChainTax(tax) {
            log3(0, 0, 0x6e9f2cb42838841da92788e02012c2b71239e040f7b2291e5b200ac8c7c3b925, getContextUser(), tax)
        }
        
        // Emit HomewordRendered event log
        mstore(0x300, len)
        log2(0x300, 32, 0x50fdb39d7be5a0438cf1dfba392019ff88d8b634882dfba51912bc5527e0291f, getContextUser())
        
        sstore(getUserSlot(55014), 1) // Status: Success
        sstore(getUserSlot(55013), 0) // Clear strobe
    }
}
```
