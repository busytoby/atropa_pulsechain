# Commodore 64 VIC-II Hardware Tricks — Raster Interrupts & Sprite Multiplexing

The **MOS Technology 6567/6569 (VIC-II)** video controller was the technical heart of the Commodore 64. Its architectural limits—specifically its hard boundary of displaying only 8 hardware sprites simultaneously—forced developers to invent advanced low-level tricks to bypass system specifications. 

This analysis details how developers manipulated VIC-II registers at the machine level, focusing on **raster interrupts** and **sprite multiplexing**, and provides a Yul-based emulator of a sprite multiplexer scheduling queue.

---

## 1. Low-Level VIC-II Register Manipulation

### 1.1 Cycle-Accurate Raster Interrupts (`$D011` & `$D012`)
The VIC-II tracks the current scanline of the electron beam. 
* **The Registers:** `$D012` holds the low 8 bits of the active scanline (0–255), while Bit 7 of `$D011` functions as the 9th bit to cover the PAL limit of 312 lines.
* **The IRQ Trigger:** By writing a line number to `$D012` and enabling the raster interrupt mask (Bit 0 of `$D01A`), the VIC-II halts the 6510 CPU at the exact moment the beam reaches that line.
* **Acknowledge:** The interrupt handler must write `1` to Bit 0 of `$D019` to clear the latch before the next scanline trigger.

### 1.2 Sprite Multiplexing (VIC-II Channel Reuse)
To render more than 8 sprites on screen, developers reused the 8 hardware sprite registers ($D000–$D00F) as the beam traveled vertically:
1. **Y-Sorting:** The system sorts all "virtual" sprite coordinates by their vertical ($Y$) position.
2. **Scanline Handshake:** A raster interrupt is set at line $Y_i$.
3. **Register Swapping:** When the interrupt triggers, the CPU swaps the hardware registers of the sprites that just finished drawing (Y-coordinate + height of 21 scanlines) with the coordinates and data pointers of the next virtual sprites.
4. **Reschedule:** The next interrupt is scheduled at scanline $Y_{i+1}$.

```
  Scanline (Y)
   Line 50: [Physical Sprite 0] draws Virtual Sprite A
   Line 70: [Physical Sprite 0] finished.
            --> Raster IRQ triggers. Registers for Sprite 0 updated with Virtual Sprite B.
   Line 90: [Physical Sprite 0] draws Virtual Sprite B further down.
```

---

## 2. On-Chain Emulation Schema: Virtual Sprite Multiplexer

To simulate this VIC-II multiplexer scheduling loop in Yul, we sort virtual sprites by Y-coordinate and map them to the 8 physical hardware slots.

### 2.1 Virtual Sprite Layout (Calldata Input)
Each virtual sprite is represented by a single packed `uint256` word:
* **Bits 0 - 15:** Virtual Sprite ID
* **Bits 16 - 31:** Sprite X Coordinate
* **Bits 32 - 47:** Sprite Y Coordinate
* **Bits 48 - 63:** Pointer/Texture ID

### 2.2 Yul Multiplexer Scheduler
The following Yul function acts as a scheduling resolver. It takes a list of virtual sprites, sorts them by Y-coordinate, and maps them to physical sprite channels, outputting the raster line interrupt trigger points.

```yul
// Method 19: scheduleVirtualSprites(numVirtualSprites, virtualSprites...)
// Selector: 0x6d1b8fa2
if eq(selector, 0x6d1b8fa2) {
    let numSprites := calldataload(4)
    if gt(numSprites, 16) { revert(0, 0) } // Cap at 16 virtual sprites for stack limits

    // 1. Load sprites into memory for sorting
    // Memory offsets: 0x00 to 0x1f0 (32 bytes per sprite)
    for { let i := 0 } lt(i, numSprites) { i := add(i, 1) } {
        let word := calldataload(add(36, mul(i, 32)))
        mstore(mul(i, 32), word)
    }

    // 2. Perform Bubble Sort on Y-coordinate (stored at bits 32-47 of each word)
    for { let i := 0 } lt(i, sub(numSprites, 1)) { i := add(i, 1) } {
        for { let j := 0 } lt(j, sub(sub(numSprites, i), 1)) { j := add(j, 1) } {
            let spriteA := mload(mul(j, 32))
            let spriteB := mload(mul(add(j, 1), 32))

            let yA := and(shr(32, spriteA), 0xffff)
            let yB := and(shr(32, spriteB), 0xffff)

            if gt(yA, yB) {
                // Swap memory slots
                mstore(mul(j, 32), spriteB)
                mstore(mul(add(j, 1), 32), spriteA)
            }
        }
    }

    // 3. Map virtual sprites to 8 physical registers down the Y-axis
    // physicalYOffset[8] stores the Y scanline where each physical slot becomes free (lastY + 21)
    // Initialize physical slots to 0
    let mOffset := 0x200
    for { let p := 0 } lt(p, 8) { p := add(p, 1) } {
        mstore(add(mOffset, mul(p, 32)), 0)
    }

    // output buffer starts at 0x300. We write the physical register assignments:
    // (virtualSpriteId, physicalRegisterIndex, rasterTriggerLine)
    let outOffset := 0x300
    let scheduledCount := 0

    for { let i := 0 } lt(i, numSprites) { i := add(i, 1) } {
        let vSprite := mload(mul(i, 32))
        let vId := and(vSprite, 0xffff)
        let vX := and(shr(16, vSprite), 0xffff)
        let vY := and(shr(32, vSprite), 0xffff)

        // Find the first free physical register slot
        let assignedSlot := 9
        for { let p := 0 } lt(p, 8) { p := add(p, 1) } {
            let freeY := mload(add(mOffset, mul(p, 32)))
            // If physical slot is free at or before the virtual sprite's Y coordinate
            if iszero(gt(freeY, vY)) {
                assignedSlot := p
                p := 8 // break loop
            }
        }

        // If a physical register was found (assignedSlot < 8)
        if lt(assignedSlot, 8) {
            // Update the physical register's free scanline (vY + 21 scanlines height)
            mstore(add(mOffset, mul(assignedSlot, 32)), add(vY, 21))

            // Write assignment details to output
            mstore(add(outOffset, mul(scheduledCount, 96)), vId)
            mstore(add(add(outOffset, mul(scheduledCount, 96)), 32), assignedSlot)
            mstore(add(add(outOffset, mul(scheduledCount, 96)), 64), vY)

            scheduledCount := add(scheduledCount, 1)
        }
    }

    // Return the scheduled virtual sprite timeline
    return(outOffset, mul(scheduledCount, 96))
}
```

---

## 3. Physical Limits & Safety Rules

When utilizing the on-chain virtual scheduler or programming the physical VIC-II:
1. **Horizontal Contention:** No more than 8 virtual sprites can share the same horizontal band. If a 9th sprite is scheduled at the same Y-level, `assignedSlot` returns `9` (exceeds registers) and the sprite is dropped from the active frame timeline.
2. **Cycle Overhead:** The interrupt routine must execute within the horizontal blanking period (approx 40 CPU cycles) to avoid "shearing" or sprite tearing glitches.

---

## 4. ZMM VM Verification

The Yul-based sprite multiplexer scheduler has been fully verified inside the C-based ZMM Virtual Machine test suite ([test_zmm_fighter.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_zmm_fighter.c)):

* **Test Input Parameters**:
  * `numSprites` = 5
  * Sprite 1: ID = 1, Y = 50, X = 100
  * Sprite 2: ID = 2, Y = 10, X = 100
  * Sprite 3: ID = 3, Y = 30, X = 100
  * Sprite 4: ID = 4, Y = 70, X = 100
  * Sprite 5: ID = 5, Y = 90, X = 100

* **Sorted & Assigned Physical Channel Timeline (Height = 21)**:
  1. **Sprite 2 (Y=10)** -> Assigned **Physical Slot 0** (Free scanline: 31).
  2. **Sprite 3 (Y=30)** -> Slot 0 is busy until 31. Assigned **Physical Slot 1** (Free scanline: 51).
  3. **Sprite 1 (Y=50)** -> Slot 0 is free (31 <= 50). Assigned **Physical Slot 0** (Free scanline: 71).
  4. **Sprite 4 (Y=70)** -> Slot 0 is busy until 71; Slot 1 is free (51 <= 70). Assigned **Physical Slot 1** (Free scanline: 91).
  5. **Sprite 5 (Y=90)** -> Slot 0 is free (71 <= 90). Assigned **Physical Slot 0** (Free scanline: 111).

All assertions passed successfully, demonstrating correct physical register reassignment across scanlines without horizontal overlaps.

---

> [!TIP]
> The sorting algorithm is the critical phase. Without sorting virtual sprites by Y-coordinate, the raster interrupt routine cannot progress sequentially down the screen, causing the VIC-II to miss triggers and drop entire sprite groups.

