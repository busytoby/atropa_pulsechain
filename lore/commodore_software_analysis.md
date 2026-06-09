# Commodore Software — Technical & Physics Analysis

Unlike dedicated game companies, **Commodore** (Commodore Business Machines) did not maintain a large internal game development studio. Instead, the hardware manufacturer functioned as a publisher and distributor, hiring independent contractors to build key software titles. This strategy served to fill market gaps and showcase the advanced graphical capabilities (sprites, raster interrupts) of the VIC-II chip and the audio capabilities (ADSR filters) of the SID chip to consumers.

This document details the mechanics of Commodore's flagship software releases, specifically ***International Soccer*** (1983) by Andrew Spencer and ***Simons' BASIC*** (1983) by David Simons, exploring how their physics models map to low-level assembly paradigms.

---

## 1. Flagship Technical Innovations

### 1.1 International Soccer (1983)
Developed by Andrew Spencer, *International Soccer* was a landmark sports game praised for its fluid animation, side-perspective scrolling, and advanced physics:
* **True 3D Bouncing Ball Physics:** The game modeled gravity ($g$) and vertical displacement ($Z$) independently of the 2D playing field coordinates $(X, Y)$:
  - Height vector calculation:
    $$Z(t) = Z_0 + v_{z0} t - \frac{1}{2} g t^2$$
  - Ground bounce contact reflection: when $Z \le 0$, the vertical velocity reflected with a coefficient of restitution ($e \approx 0.7$):
    $$v_z' = -e \cdot v_z$$
* **The "Head-Carry" Physics Glitch:** If the ball’s horizontal position $(X, Y)$ and vertical height ($Z$) aligned with a player’s forehead bounding box while sprinting, the game engine locked the ball's coordinates directly to the player's head movement vectors:
  $$X_{\text{ball}} = X_{\text{player}} + \text{offset}_x$$
  $$Z_{\text{ball}} = H_{\text{player}}$$
  Andrew Spencer deliberately left this bug in the game as it added a fun, signature dribbling mechanic.

### 1.2 Simons' BASIC (1983)
Written by 16-year-old David Simons, this cartridge extension added 114 new commands to the Commodore 64's standard BASIC 2.0:
* **Hardware Register Abstraction:** Simons' BASIC bypassed slow standard command parser execution by mapping high-level instructions (like `HIRES`, `LINE`, `SPRITE`) directly to assembler macro sequences that read/wrote to the VIC-II register map (`$D000 - $D02E`), democratizing high-performance C64 programming.

---

## 2. On-Chain Emulation Schema: Ball Bouncing & Head-Carry

To model the bouncing ball kinematics and the "Head-Carry" state lock in Yul, we map the physics integration step below.

### 2.1 Physics Registers
| Memory Slot | Register Name | Data Range | Description |
| :---: | :--- | :--- | :--- |
| `55500` | **Ball X** | `0 - 320` | Ball horizontal coordinate |
| `55501` | **Ball Y** | `0 - 200` | Ball depth coordinate |
| `55502` | **Ball Z (Height)** | `0 - 100` | Ball height off ground |
| `55503` | **Velocity Z** | `-127 to 127` | Vertical speed vector |
| `55504` | **Player X** | `0 - 320` | Active player horizontal coordinate |
| `55505` | **Player Y** | `0 - 200` | Active player depth coordinate |
| `55506` | **Head Carry Flag** | `0 - 1` | `1` = Ball is glued to player's forehead |

### 2.2 Ball Physics Integration & Lock Loop in Yul
The following Yul function computes gravity damping, rebound bounces, and evaluates whether the ball snaps into the player's "Head-Carry" state:

```yul
// Integrates ball gravity, ground bounce, and Shamus-style head-carry locking
function updateBallSoccerState(bx, by, bz, vz, px, py, headCarry) -> newBx, newBy, newBz, newVz, newCarry {
    newBx := bx
    newBy := by
    newBz := bz
    newVz := vz
    newCarry := headCarry

    // Constants
    let gravity := 1
    let playerHeight := 24 // Head coordinate height
    let restitution := 70  // 0.7 coefficient scaled by 100

    // 1. Evaluate Head-Carry Snap Logic
    let xDiff := 0
    if gt(bx, px) { xDiff := sub(bx, px) }
    if iszero(gt(bx, px)) { xDiff := sub(px, bx) }
    
    let yDiff := 0
    if gt(by, py) { yDiff := sub(by, py) }
    if iszero(gt(by, py)) { yDiff := sub(py, by) }

    // If ball is within 8 horizontal pixels and height is near player's head (20-28)
    let inRange := and(lt(xDiff, 8), lt(yDiff, 8))
    let heightAligned := and(iszero(lt(bz, 20)), lt(bz, 28))

    if and(inRange, heightAligned) {
        newCarry := 1 // Trigger Head-Carry Lock
    }

    // 2. Apply Physics based on state
    if newCarry {
        // Ball is locked to the player's skull vector
        newBx := px
        newBy := py
        newBz := playerHeight
        newVz := 0
    }
    
    if iszero(newCarry) {
        // Apply normal gravity vector integration
        newBz := add(bz, vz)
        newVz := sub(vz, gravity)

        // Bounce check on ground contact (bz <= 0 or wrap-around high numbers)
        if or(lt(newBz, 0), gt(newBz, 200)) {
            newBz := 0
            // Reflect vertical speed with dampening: vz' = -0.7 * vz
            let absVz := vz
            if lt(vz, 0) { absVz := sub(0, vz) }
            
            newVz := div(mul(absVz, restitution), 100)
            
            // Cease bouncing if energy is low (speed < 2)
            if lt(newVz, 2) {
                newVz := 0
            }
        }
    }
}
```

---

## 3. Comparative Chronology

| Aspect | Synapse Software | Commodore Software Publications |
| :--- | :--- | :--- |
| **Active Era** | 1981 - 1985 | 1982 - 1994 (Commodore bankruptcy) |
| **Origin** | San Francisco Bay Area, US | West Chester, Pennsylvania, US |
| **Strategic Goal** | High-concept artistic game publishing | Hardware showcase & cartridge bundling |
| **Technical Focus** | Variable-vector flight & parser states | Sprites, raster interrupts, memory mapping |
| **Signature Titles** | *Fort Apocalypse*, *Shamus*, *Alley Cat* | *International Soccer*, *Simons' BASIC* |

---

> [!TIP]
> Andrew Spencer's head-carry bug illustrates the value of "emergent gameplay mechanics" in early game design. By leaving the glitch in, Spencer allowed players to develop advanced dribbling strategies, transforming *International Soccer* into a competitive multiplayer classic.
