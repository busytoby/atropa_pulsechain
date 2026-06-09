# Mystery House Vector Engine & Z-Machine On-Chain Integration

This document outlines the design architecture and Yul implementation for bridging the **Z-Machine parser** and **Mystery House vector graphic engine** on-chain, rendering scenes to Vulkan staging textures.

---

## 1. Z-Machine Vector Interface: getVectorScene ($2e06cbe8)

We introduce a dedicated graphical vector query method inside the on-chain Z-Machine interpreter:

```solidity
interface IZMachine {
    // Returns the vector drawing command stream for a specific adventure game room
    function getVectorScene(uint256 roomIndex) external view returns (bytes memory);
}
```

The returned byte array is structured as a series of 5-byte line drawing segments:
* **Byte 0**: Start $X$ coordinate (0–255).
* **Byte 1**: Start $Y$ coordinate (0–255).
* **Byte 2**: End $X$ coordinate (0–255).
* **Byte 3**: End $Y$ coordinate (0–255).
* **Byte 4**: Line Color index (0–15).

---

## 2. Rendering Pipeline Coordination (Vulkan & DeepSeek)

When the player navigates or enters a room:
1. The client queries the Z-Machine contract using `getVectorScene(roomIndex)`.
2. The returned bytes are parsed and drawn onto the C64 Doodle Graphics framebuffer via the Yul CPU emulator.
3. The Doodle framebuffer updates are uploaded to the GPU via Vulkan pipelines, synchronizing visual adventure rooms.
4. The transaction logs are hashed and verified against the DeepSeek consensus status bar to guarantee mathematical compliance.
