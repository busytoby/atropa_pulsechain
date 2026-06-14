# Advanced Retro Graphics Improvements from Dr. Dobb's Journal

This document outlines the mathematical model and Yul integration designs for two highly requested graphics algorithms from *Dr. Dobb's Journal*: **Xiaolin Wu's Fast Antialiasing** and **Floyd-Steinberg Dithering**. These algorithms allow the **TSFi2 graphicsSystem.yul** contract to output smooth, high-fidelity visuals on simulated low-resolution retro framebuffers.

---

## 1. Xiaolin Wu's Fast Antialiased Lines

Unlike Bresenham's algorithm, which plots only binary pixel states (causing staircasing or "jaggies"), Xiaolin Wu's algorithm plots two pixels at each step along the line, with intensities proportional to the line's distance from the pixel centers:

```
                  (x, y)     intensity = 1 - fractional_part
      Line ------>  *
                  (x, y+1)   intensity = fractional_part
```

This simulates sub-pixel precision on integer raster displays.

### Yul Implementation Design for Xiaolin Wu's Algorithm

In Yul, we use fixed-point math ($1000$ scale) to interpolate coordinates and transparency:

```yul
// Method 36: drawWuLine(int256 x1, int256 y1, int256 x2, int256 y2, uint8 colorIndex, uint256 baseAddr)
// Selector: 0xd7f2fa88
if eq(selector, 0xd7f2fa88) {
    let x1 := calldataload(4)
    let y1 := calldataload(36)
    let x2 := calldataload(68)
    let y2 := calldataload(100)
    let color := and(calldataload(132), 0xFF)
    let baseAddr := calldataload(164)

    let dx := sub(x2, x1)
    let dy := sub(y2, y1)

    // Handle horizontal/vertical lines using simple Bresenham fallback
    if or(eq(dx, 0), eq(dy, 0)) {
        drawLinearFallback(x1, y1, x2, y2, color, baseAddr)
        leave
    }

    // Determine coordinate slopes (fixed-point scale 1000)
    let steep := 0
    if gt(abs(dy), abs(dx)) {
        steep := 1
        // Swap X and Y for steep lines
        let tmp := x1; x1 := y1; y1 := tmp
        tmp := x2; x2 := y2; y2 := tmp
    }

    if gt(x1, x2) {
        let tmp := x1; x1 := x2; x2 := tmp
        tmp := y1; y1 := y2; y2 := tmp
    }

    dx := sub(x2, x1)
    dy := sub(y2, y1)
    let gradient := sdiv(mul(dy, 1000), dx)

    // Handle first endpoint
    let xend := x1
    let yend := y1
    let xpxl1 := xend
    let ypxl1 := yend

    plotWuPixel(xpxl1, ypxl1, color, 1000, steep, baseAddr)

    let intery := add(y1, gradient) // y-intersection coordinate

    // Main step loop
    for { let x := add(xpxl1, 1) } le(x, x2) { x := add(x, 1) } {
        let iparty := sdiv(intery, 1000)
        let fparty := mod(intery, 1000)

        // Plot two adjacent pixels with complementary intensities
        plotWuPixel(x, iparty, color, sub(1000, fparty), steep, baseAddr)
        plotWuPixel(x, add(iparty, 1), color, fparty, steep, baseAddr)

        intery := add(intery, gradient)
    }
    
    mstore(0x00, 1)
    return(0x00, 32)
}

function plotWuPixel(x, y, color, intensity, steep, baseAddr) {
    // Modify intensity (alpha blending simulation with background palette)
    let blendedColor := calculateAlphaBlend(color, intensity)
    
    let px := x
    let py := y
    if steep {
        px := y
        py := x
    }
    
    let addr := add(baseAddr, add(mul(py, 320), px))
    mstore8(addr, blendedColor)
}

function le(x, y) -> r {
    r := iszero(sgt(x, y))
}
```

---

## 2. Floyd-Steinberg Error Diffusion Dithering

When converting a full-color 24-bit image to a restricted palette (e.g. the 16-color Commodore 64 palette or Atari 256 colors), simply rounding to the nearest color creates harsh banding artifacts. Floyd-Steinberg dithering diffuses the quantization error to neighboring pixels according to a specific kernel matrix:

```
          * (current pixel)    7/16 (Right)
 3/16 (Down-Left)  5/16 (Down) 1/16 (Down-Right)
```

This creates a smooth, visual mix of colors that tricks the eye into seeing intermediate shades.

### Yul Implementation Design for Floyd-Steinberg

```yul
// Method 37: ditherFramebuffer(uint256 baseAddr, uint16 width, uint16 height)
// Selector: 0xf58dbca0
if eq(selector, 0xf58dbca0) {
    let baseAddr := calldataload(4)
    let width := calldataload(36)
    let height := calldataload(68)

    for { let y := 0 } lt(y, height) { y := add(y, 1) } {
        for { let x := 0 } lt(x, width) { x := add(x, 1) } {
            let offset := add(baseAddr, add(mul(y, width), x))
            let oldColor := and(mload(offset), 0xFF)
            
            // Find closest palette match
            let newColor := findClosestPaletteMatch(oldColor)
            mstore8(offset, newColor)
            
            // Quantization error
            let error := sub(oldColor, newColor)
            
            // Diffuse error to neighbors
            // 1. Right neighbor (x + 1, y): add error * 7/16
            diffuseError(add(offset, 1), mul(error, 7), 16)
            
            // 2. Bottom-left neighbor (x - 1, y + 1): add error * 3/16
            diffuseError(sub(add(offset, width), 1), mul(error, 3), 16)
            
            // 3. Bottom neighbor (x, y + 1): add error * 5/16
            diffuseError(add(offset, width), mul(error, 5), 16)
            
            // 4. Bottom-right neighbor (x + 1, y + 1): add error * 1/16
            diffuseError(add(add(offset, width), 1), error, 16)
        }
    }
    
    mstore(0x00, 1)
    return(0x00, 32)
}

function diffuseError(pixelAddr, scaledError, divisor) {
    let currentVal := and(mload(pixelAddr), 0xFF)
    let errorTerm := sdiv(scaledError, divisor)
    let newVal := add(currentVal, errorTerm)
    
    // Clamp between 0 and 255
    if slt(newVal, 0) { newVal := 0 }
    if sgt(newVal, 255) { newVal := 255 }
    
    mstore8(pixelAddr, newVal)
}
```

---

## 3. Conclusion

Integrating Xiaolin Wu's antialiasing and Floyd-Steinberg dithering directly into our Yul graphics system provides high-fidelity, smooth vector drawing and optimal color representations. These enhancements scale our simulation graphics to match the quality standards of professional vintage software engines.
