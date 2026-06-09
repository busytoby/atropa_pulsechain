# Quicksilva, Inc. — Technical & Mechanical Analysis

Founded in Southampton in 1980 by **Nick Lambert**, **Quicksilva** was one of the earliest and most influential British software publishers of the 8-bit microcomputer boom. Operating in a highly experimental era, the company pioneered game cover storytelling, character licensing (*The Snowman*, *Strontium Dog*), and publishing high-performance titles for the Sinclair ZX80/81, ZX Spectrum, BBC Micro, and Commodore 64. 

This analysis reviews Quicksilva's legendary technical milestones, specifically Sandy White's ***Ant Attack*** (1983) and Indescomp's licensed hit ***Bugaboo (The Flea)*** (1983), and demonstrates how early isometric rendering and physics translate to low-level assembly environments.

---

## 1. Flagship Technical Innovations

### 1.1 Isometric "Soft Solid 3D" in *Ant Attack* (1983)
Created by Sandy White, *Ant Attack* was a revolutionary milestone in 3D gaming. It introduced the concept of an isometric viewpoint with true depth mechanics, allowing players to move freely in three dimensions.
* **The Projection Mechanics:** The game maps a 3D grid $(X, Y, Z)$ into a 2D isometric representation on the ZX Spectrum’s screen.
  - Standard isometric coordinates project $X$ and $Y$ at $30^\circ$ angles, and $Z$ vertically:
    $$x_{\text{screen}} = (x - y) \cdot \cos(30^\circ)$$
    $$y_{\text{screen}} = (x + y) \cdot \sin(30^\circ) - z$$
* **Structural Raycasting/Depth Buffer:** Because the ZX Spectrum lacked hardware sprite scaling or deep buffers, Sandy White developed a custom rasterization routine that drew blocks from back-to-front (Painter's Algorithm) to handle occlusions and hidden surfaces within just 48KB of RAM.
* **Rescuing State Machine:** Players climb structural blocks, jump across heights, and carry victims while dodging giant ants, requiring advanced coordinate collision checks across three dimensions.

### 1.2 Variable-Impulse Jumping in *Bugaboo (The Flea)* (1983)
Licensed from Spanish developer Indescomp, *Bugaboo* featured physics-based jumping:
* **Charge-Impulse Mechanic:** The player does not jump with a uniform force. Instead, holding the jump key charges an elastic potential energy register ($E_{\text{elastic}}$).
* **Release Trajectory:** Upon release, the accumulated energy is converted into initial velocity ($v_0$) at a fixed launch angle ($\theta$).
* **predator avoidance:** The player navigates vertical caverns while avoiding a pterodactyl predator, calculating paths and timing launches.

---

## 2. On-Chain Emulation Schema

To model these 3D isometric projections and variable jump impulses on-chain, we can layout coordinate transformation registers in Yul.

### 2.1 Coordinate Translation Registers
| Memory Slot | Register Name | Data Range | Description |
| :---: | :--- | :--- | :--- |
| `55100` | **3D Position X** | `0 - 255` | 3D space X coordinate |
| `55101` | **3D Position Y** | `0 - 255` | 3D space Y coordinate |
| `55102` | **3D Position Z** | `0 - 255` | Height coordinate |
| `55103` | **Screen X** | `0 - 320` | Computed 2D screen X coordinate |
| `55104` | **Screen Y** | `0 - 200` | Computed 2D screen Y coordinate |
| `55105` | **Elastic Charge** | `0 - 100` | Current jump energy |

### 2.2 Isometric Coordinate Projector in Yul
The following Yul function transforms 3D grid points to 2D screen space coordinates using fixed-point approximations of trigonometric ratios ($\sin(30^\circ) = 0.5$, $\cos(30^\circ) \approx 0.866$):

```yul
// Projects 3D spatial points (x, y, z) to 2D screen space (scrX, scrY)
function projectIsometric(x, y, z) -> scrX, scrY {
    let scale := 1000 // Fixed-point scaling factor
    
    // cos(30 deg) * 1000 = 866
    let cos30 := 866
    // sin(30 deg) * 1000 = 500
    let sin30 := 500
    
    // Offset constants to center the projection on a 256x192 ZX Spectrum screen
    let xOffset := 128
    let yOffset := 96
    
    // scrX = (x - y) * cos(30) + xOffset
    let diff := 0
    let isNegativeX := lt(x, y)
    if isNegativeX {
        diff := sub(y, x)
    }
    if iszero(isNegativeX) {
        diff := sub(x, y)
    }
    
    let rotatedX := div(mul(diff, cos30), scale)
    if isNegativeX {
        scrX := sub(xOffset, rotatedX)
    }
    if iszero(isNegativeX) {
        scrX := add(xOffset, rotatedX)
    }
    
    // scrY = (x + y) * sin(30) - z + yOffset
    let sumVal := add(x, y)
    let rotatedY := div(mul(sumVal, sin30), scale)
    
    scrY := sub(add(rotatedY, yOffset), z)
}
```

---

## 3. Comparative Chronology

| Aspect | Xonox | Quicksilva, Inc. |
| :--- | :--- | :--- |
| **Active Era** | 1983 | 1980 - 1990 (sold in 1984) |
| **Origin** | North America (Minnesota) | United Kingdom (Southampton) |
| **Platform Specialty** | Atari 2600, ColecoVision | Sinclair ZX Spectrum, Commodore 64 |
| **Primary Innovation** | Double-Ender Cartridges | Isometric "Soft Solid 3D", Storytelling covers |
| **Key Titles** | *Artillery Duel*, *Chuck Norris Superkicks* | *Ant Attack*, *Bugaboo (The Flea)*, *Time-Gate* |
| **Simulation Core** | Wind-drift ballistic paths | Depth sorting, Occlusion projection, Variable-charge jumps |

---

> [!TIP]
> Sandy White's Painter's Algorithm in *Ant Attack* is a masterclass in graphics optimization. By calculating screen offsets in a single pass without complex sorting trees, the engine achieved playable 3D rendering speeds on a 3.5 MHz Z80 CPU.
