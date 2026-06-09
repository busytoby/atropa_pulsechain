# Folklore CPU Architecture & Design Specification

This document details the architectural design for the **Folklore CPU**, an EVM-native state engine written in Yul. By utilizing native EVM/Yul characteristics, we eliminate the emulation overhead of the 6502 interpreter while fully leveraging modern virtual machine capabilities.

---

## 1. Core EVM-Native Advantages

Unlike retro processors limited to 8-bit registers and 64KB memory addresses, the **Folklore CPU** runs directly on the virtual machine using:
1. **256-bit Word Size (`u256`)**: Coordinates, speed vectors, timers, and health states are stored as native 256-bit EVM words. This eliminates complex multi-byte arithmetic (like high/low byte carry logic) for coordinates and physics.
2. **Infinite Storage Slots (`sstore`/`sload`)**: Instead of dividing a 64KB RAM layout, we use the EVM's $2^{256}$ storage slot keys to structure game variables cleanly using structured hashing (e.g., standard mapping offsets).
3. **Native Math & Bitwise Opcodes**: Direct access to `addmod`, `mulmod`, `exp`, `signextend`, and fast bitwise shifting, allowing high-performance procedural generation, collision math, and physics.

---

## 2. Register & Storage Map

The CPU state is organized directly in EVM storage slots. Below is the layout designed for the Vaesen-themed game logic:

| Storage Key (u256 Slot) | Variable Name | Type | Description |
| :--- | :--- | :--- | :--- |
| `0x00` | `magic_selector` | `u256` | Active instruction / state update routine (e.g., tick, reset) |
| `0x01` | `player_x` | `u256` | Player horizontal position (scaled by 1000 for decimal precision) |
| `0x02` | `player_y` | `u256` | Player vertical position (scaled by 1000) |
| `0x03` | `player_vy` | `u256` | Player vertical velocity |
| `0x04` | `player_jumping` | `u256` | Jump boolean flag (0 = Ground, 1 = Jumping, 2 = Double Jump) |
| `0x05` | `player_energy` | `u256` | Current player energy / stamina (0–100) |
| `0x06` | `physical_trauma`| `u256` | Vaesen physical trauma level (0 = Normal to 3 = Broken) |
| `0x07` | `mental_trauma` | `u256` | Vaesen mental trauma level (0 = Normal to 3 = Panicked) |
| `0x10` | `enemy_count` | `u256` | Number of active enemies / obstacles |
| `0x20 + i` | `enemy_x[i]` | `u256` | Horizontal position of enemy $i$ |
| `0x40 + i` | `enemy_y[i]` | `u256` | Vertical position of enemy $i$ |
| `0x60 + i` | `enemy_vx[i]` | `u256` | Velocity of enemy $i$ |

---

## 3. ABI Selection Protocol

The C harness interacts with the Yul contract using a standardized 4-byte EVM function selector passed in the transaction call data:

```solidity
switch selector()
// 0x6e559e22: reset() -> Clear storage, set initial state coordinates
case 0x6e559e22 {
    sstore(0x01, 100000)  // player_x = 100.00
    sstore(0x02, 520000)  // player_y = 520.00
    sstore(0x05, 100)     // energy = 100
    // ...
}
// 0x9c3d42b0: tick(moveDir, jumpTrigger) -> Process physics tick
case 0x9c3d42b0 {
    let moveDir := calldataload(4)
    let jumpTrigger := calldataload(36)
    processPhysics(moveDir, jumpTrigger)
}
// 0x2e0f40cf: get_game_state() -> Returns all registers in single return payload
case 0x2e0f40cf {
    mstore(0x00, sload(0x01)) // player_x
    mstore(0x20, sload(0x02)) // player_y
    mstore(0x40, sload(0x05)) // energy
    return(0x00, 0x60)
}
```

---

## 4. Native Physics & Gravity implementation (Yul snippet)

This snippet shows how simple and readable gravity and physics updates become when using native Yul rather than emulated assembly:

```solidity
function processPhysics(moveDir, jumpTrigger) {
    // 1. Gravity and Jump logic
    let py := sload(0x02)
    let pvy := sload(0x03)
    let jumping := sload(0x04)
    
    if jumpTrigger {
        if iszero(jumping) {
            pvy := -15000 // Native jump impulse (upwards)
            jumping := 1
        }
    }
    
    // Apply gravity
    pvy := add(pvy, 800) // Gravity constant
    py := add(py, pvy)
    
    // Floor boundary check (520.00)
    if gt(py, 520000) {
        py := 520000
        pvy := 0
        jumping := 0
    }
    
    sstore(0x02, py)
    sstore(0x03, pvy)
    sstore(0x04, jumping)
}
```

---

## 5. High-Color (Hi-Color) Sprite Graphics Extensions

To support complex high-color graphics without the rendering constraints of traditional 8-bit registers, the Folklore CPU architecture can be upgraded with native 8-bit color-depth sprite patterns and dynamic palette composition.

### 5.1 Native 8-Bit Color-Depth Grid Packing
Traditional C64 registers divide rows into 1-bit or 2-bit color cells. In contrast, the Folklore CPU leverages native `u256` storage slots to pack full **8-bit color indices** (supporting up to 256 colors per pixel):
* A sprite row width of 24 pixels requires exactly 24 bytes (192 bits).
* A single 256-bit word easily accommodates 24 bytes, leaving 64 bits free for row-level effects (such as alpha values or horizontal flipping flags).
* Thus, a 24x21 hi-color sprite fits into exactly **21 storage slots** (one slot per row):
  $$\text{Storage Key} = \text{SpriteBase} + \text{RowIndex}$$

### 5.2 Dynamic RGBA Palette Lookups
Rather than hardcoding standard system colors, the Folklore CPU can store customizable color lookup tables (LUTs) in EVM storage:
* **Palette Storage Slots (`0x1000` to `0x10FF`)**: Each slot stores a 32-bit RGBA color representation (`0xRRGGBBAA`).
* **Palette Selection Register (`0x08`)**: Swaps active palettes globally or per-sprite (supporting dynamic daytime/nighttime coloring, damage flashes, or shield overlays).

### 5.3 On-Chain Procedural Shaders
Using Yul mathematical arithmetic opcodes, the Folklore CPU can execute procedural shader functions on sprite frames prior to output:
```solidity
// Example: Apply linear horizontal lighting gradient on a sprite row
function applyLightingGradient(rowVal, ambientLight) -> shadedRow {
    for { let pixel := 0 } lt(pixel, 24) { pixel := add(pixel, 1) } {
        // Extract 8-bit color index
        let colorIdx := and(shr(mul(pixel, 8), rowVal), 0xFF)
        
        // Procedurally scale color index values based on position
        let lightLevel := add(ambientLight, mul(pixel, 8))
        if gt(lightLevel, 255) { lightLevel := 255 }
        
        // Write back shaded pixel
        let shadedPixel := div(mul(colorIdx, lightLevel), 255)
        shadedRow := or(shadedRow, shl(mul(pixel, 8), shadedPixel))
    }
}
```

### 5.4 Sprite Composition & Alpha Blending
* **Z-Buffer/Depth Registry (`0x09`)**: Assigns depth levels to each active sprite, sorting paint operations dynamically.
* **Alpha Control Register (`0x0A`)**: Regulates translucency and overlay blending math (standard linear interpolation $C = \alpha A + (1-\alpha) B$) processed natively in Yul arithmetic routines.

