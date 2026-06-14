# Integrating Bresenham's Line Algorithm from Dr. Dobb's Journal

This document details the mathematical model and Yul integration of **Bresenham's Line Drawing Algorithm** (heavily optimized and discussed in *Dr. Dobb's Journal* graphics columns) into the **TSFi2 graphicsSystem.yul** contract.

---

## 1. The Mathematical Model of Bresenham's Algorithm

To draw a line from $(x_1, y_1)$ to $(x_2, y_2)$ on a raster grid without floating-point math, we compute the slope error relative to the grid lines. Let:
*   $\Delta x = |x_2 - x_1|$
*   $\Delta y = |y_2 - y_1|$
*   $s_x = \text{sign}(x_2 - x_1)$
*   $s_y = \text{sign}(y_2 - y_1)$

The decision parameter is initialized as:
$$D = 2\Delta y - \Delta x \quad (\text{assuming } \Delta x \ge \Delta y)$$

At each coordinate step along the dominant axis (e.g., $X$):
*   Plot $(x, y)$.
*   If $D > 0$:
    *   $y := y + s_y$
    *   $D := D + 2(\Delta y - \Delta x)$
*   Else:
    *   $D := D + 2\Delta y$
*   $x := x + s_x$

This entire process uses only integer additions, subtractions, and bit shifts, making it extremely gas-efficient in Yul.

---

## 2. Yul Implementation for `graphicsSystem.yul`

Below is the proposed implementation of **Method 35: drawBresenhamLine** for `graphicsSystem.yul`. It plots pixel colors directly into a virtual framebuffer address using Bresenham's algorithm:

```yul
// Method 35: drawBresenhamLine(int256 x1, int256 y1, int256 x2, int256 y2, uint8 color, uint256 baseAddr)
// Selector: 0xb9a31a9f
if eq(selector, 0xb9a31a9f) {
    let x1 := calldataload(4)
    let y1 := calldataload(36)
    let x2 := calldataload(68)
    let y2 := calldataload(100)
    let color := and(calldataload(132), 0xFF)
    let baseAddr := calldataload(164)

    let dx := abs(sub(x2, x1))
    let dy := abs(sub(y2, y1))
    
    let sx := 1
    if lt(x2, x1) { sx := -1 }
    
    let sy := 1
    if lt(y2, y1) { sy := -1 }
    
    let err := sub(dx, dy)
    
    let cx := x1
    let cy := y1
    
    // Draw loop
    for { } 1 { } {
        // Plot pixel (assuming linear framebuffer with screen width 320)
        let pixelAddr := add(baseAddr, add(mul(cy, 320), cx))
        mstore8(pixelAddr, color)
        
        // Break if we reached the target endpoint
        if and(eq(cx, x2), eq(cy, y2)) { break }
        
        let e2 := mul(err, 2)
        
        // Adjust error for X axis
        if sgt(e2, sub(0, dy)) {
            err := sub(err, dy)
            cx := add(cx, sx)
        }
        
        // Adjust error for Y axis
        if slt(e2, dx) {
            err := add(err, dx)
            cy := add(cy, sy)
        }
    }
    
    mstore(0x00, 1)
    return(0x00, 32)
}

function abs(x) -> r {
    r := x
    if slt(x, 0) { r := sub(0, x) }
}
```

---

## 3. Benefits of Integration
*   **Gas Efficiency**: Zero division operations (`div`/`sdiv`), replacing them with low-gas `add`, `sub`, and logical operators.
*   **Vector Engine Independence**: Allows the guest ZMM VM program to rasterize lines and wireframe shapes without relying on the host-side HTML canvas renderer, enabling standalone 3D simulations inside the contract logic.
*   **Enhanced Compatibility**: Fits perfectly with the existing Tom Hudson `clipLine` routine to form a complete vector drawing toolkit.

---

## 4. Conclusion

By integrating Bresenham's Line Algorithm directly into `graphicsSystem.yul`, we follow *Dr. Dobb's Journal*'s core optimization philosophy of minimizing CPU overhead. This allows the TSFi2 platform to handle fast 3D graphics rasterization cleanly in Yul assembly.
