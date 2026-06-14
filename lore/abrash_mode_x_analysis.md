# Michael Abrash's Mode X Planar Addressing & TSFi2 Implementation

This document reviews Michael Abrash's **Mode X Write Pixel Routine** (`L47-2.ASM`), analyzing how planar memory organization optimized VGA graphics, and provides a Yul translation of this planar memory addressing algorithm for the **TSFi2** display subsystem.

---

## 1. Mode X Planar Memory Architecture

In standard VGA Mode 13h ($320 \times 200$, 256 colors), memory is configured in "chained" mode, where each pixel byte is laid out sequentially in RAM. While simple to address, this configuration prevents programmers from utilizing the hardware's planar architecture.

In **Mode X** ($320 \times 240$, 256 colors), the memory is "unchained" into 4 distinct planes:
*   **Plane 0** holds pixels with $X$ coordinates where $X \pmod 4 = 0$.
*   **Plane 1** holds pixels with $X$ coordinates where $X \pmod 4 = 1$.
*   **Plane 2** holds pixels with $X$ coordinates where $X \pmod 4 = 2$.
*   **Plane 3** holds pixels with $X$ coordinates where $X \pmod 4 = 3$.

To write a single pixel at coordinates $(X, Y)$:
1. **Determine Plane**: Select the destination plane using the Map Mask register:
   $$\text{Plane Mask} = 1 \text{ shl } (X \pmod 4)$$
   Write this mask value to the Map Mask register (Index 2 of the Sequencer, Port `0x3C5`).
2. **Calculate Offset**: The screen width is 320 pixels. Since 4 pixels are packed across 4 parallel planes at the same offset address, the scanline width is $320 / 4 = 80$ bytes:
   $$\text{Offset} = (Y \times 80) + \lfloor X / 4 \rfloor$$
3. **Write Color**: Write the color value to memory at $\text{BaseAddress} + \text{Offset}$.

This planar division allows for extremely fast block copying (using the VGA hardware's latches) since 4 pixels can be copied in a single memory cycle by writing to all 4 planes simultaneously.

---

## 2. Planar Pixel Plotting in Yul

We can emulate planar VGA layouts in Yul to model retro display systems. Here is a Yul function that calculates the planar address offset and plane mask, mimicking Abrash's `L47-2.ASM` logic:

```yul
// Michael Abrash's Mode X planar write pixel logic translated to Yul
// x, y represent coordinate coordinates
// color represents the 8-bit color index
// baseAddr represents the start of the simulated VGA framebuffer memory
function writePixelModeX(x, y, color, baseAddr) {
    // 1. Calculate the destination plane (x mod 4)
    let plane := and(x, 3)
    
    // Map plane index to VGA Sequencer Map Mask value (1 << plane)
    let planeMask := shl(plane, 1)
    
    // Select the plane in the simulated Sequencer port
    selectVgaPlane(planeMask)
    
    // 2. Calculate planar offset address
    // Offset = (y * 80) + (x / 4)
    // we use fast shift instead of division: x / 4 is shr(2, x)
    let yOffset := mul(y, 80)
    let xOffset := shr(2, x)
    let offset := add(yOffset, xOffset)
    
    // 3. Write pixel to simulated planar memory
    // Each simulated plane sits at a distinct memory segment
    let destinationAddress := add(baseAddr, add(offset, shl(16, plane)))
    mstore8(destinationAddress, color)
}

function selectVgaPlane(mask) {
    // Write plane mask to simulated Map Mask port (0x3C5)
    sstore(0x3c5, mask)
}
```

---

## 3. Benefits of Planar Emulation

Planar memory modeling allows the TSFi2 graphics engine to execute retro VGA programs directly within our virtual environment. By using the plane selection mask, the guest code can perform high-speed page-flipping and multi-plane fill operations (like clear screens) by writing 4-pixel patterns to all planes at once, replicating the hardware optimizations popularized by Abrash.

---

## 4. Conclusion

Michael Abrash's Mode X algorithms illustrated the power of understanding hardware mechanics to maximize performance. Implementing these planar memory mappings in the TSFi2 graphics pipeline provides authentic retro graphics execution and lets us run early 3D renderers in a verifiably accurate manner.
