# Xonox — Technical & Mechanical Analysis

Xonox, active in 1983 and operating as a boutique division of the legendary compilation giant **K-tel Software**, holds a distinct place in home console history. While their existence was cut short by the 1983 video game crash, their unique marketing/hardware gimmicks—specifically the physical "Double-Ender" cartridge—alongside tight strategic implementations like *Artillery Duel*, demonstrated highly efficient resource utilization under console constraint limits.

This document examines Xonox's technical accomplishments, mathematical models for ballistic projectile paths, and patterns for simulating hardware bank-switching and physics on-chain.

---

## 1. Technical & Hardware Innovations

### 1.1 The "Double-Ender" Cartridge
Rather than utilizing soft bank-switching during gameplay, Xonox's flagship hardware design was physical:
* **Two-in-One Cartridge:** The unit featured two independent PCB connector edges and internal ROM chips pointing to different games (e.g., *Artillery Duel* on one side, *Chuck Norris Superkicks* or *Ghost Manor* on the other).
* **Selection Mechanism:** The player selected the active game by physically flipping the cartridge upside down and inserting the opposite edge into the slot.
* **Low-Level Emulator Equivalence:** In digital emulators or smart contracts, this behaves like a static 2-in-1 layout where a state register or an offset parameter determines which instruction pipeline is executed:
  $$\text{PC}_{\text{entry}} = \text{GameSelect} \times \text{Offset}_{\text{GameB}}$$

### 1.2 Artillery Duel (1983)
A foundational entry in the turn-based artillery genre, *Artillery Duel* required the console's microprocessor (such as the MOS 6507 in the Atari 2600) to calculate ballistic trajectories over a randomly generated landscape:
* **The Inputs:** The players adjust barrel angle ($\theta$) and powder charge velocity ($v_0$).
* **Environmental Vectors:** The system dynamically selects a randomized wind velocity vector ($a_{\text{wind}}$) for each turn.
* **The Math:** The trajectory combines gravity ($g$) and horizontal wind drag:
  $$x(t) = (v_0 \cos(\theta) + a_{\text{wind}}) \cdot t$$
  $$y(t) = (v_0 \sin(\theta)) \cdot t - \frac{1}{2} g t^2$$
* **Terrain Collision:** For every time increment $\Delta t$, the system checks the coordinate $(x, y)$ against the screen's raster-defined terrain heightmap array:
  $$\text{Collision} = y(t) \le \text{Terrain}[x(t)]$$

---

## 2. On-Chain Emulation Schema

To represent Xonox's systems inside our Yul/EVM subsystem, we map the dual-game selector and the trajectory calculation code.

### 2.1 State Register Layout
| Memory Slot | Register Name | Data Range | Description |
| :---: | :--- | :--- | :--- |
| `55000` | **Game Selector** | `0 - 1` | `0` = Game A (Artillery Duel), `1` = Game B (Superkicks) |
| `55001` | **Wind Vector** | `-127 to 127` | Signed wind horizontal force |
| `55002` | **Barrel Angle** | `0 - 90` | Degrees of elevation |
| `55003` | **Powder Charge** | `0 - 100` | Initial velocity magnitude |
| `55004` | **Target X** | `0 - 320` | Enemy X coordinate |

### 2.2 Ballistic Trajectory Resolver in Yul
The following Yul function emulates the discrete steps of the artillery shell, applying gravity and wind vectors to check if it hits the ground or the target:

```yul
// Calculates a discrete ballistic step with fixed-point arithmetic (18 decimals)
function simulateStep(t, angleDeg, velocity, wind, targetX, targetY) -> hitTarget, hitTerrain {
    let scale := 1000000000000000000 // 1e18 scale for fixed point
    
    // Abstracted Sine and Cosine lookups for simplicity
    let cosVal := getCos(angleDeg) // fixed-point value
    let sinVal := getSin(angleDeg) // fixed-point value
    
    // x = (v0 * cos(theta) + wind) * t
    let termX := add(div(mul(velocity, cosVal), scale), wind)
    let x := div(mul(termX, t), scale)
    
    // y = v0 * sin(theta) * t - (0.5 * g * t^2)
    // g is approximated at 9.8 (9800000000000000000 in fixed-point)
    let gHalf := 4900000000000000000
    let termY1 := div(mul(velocity, sinVal), scale)
    let termY1_t := div(mul(termY1, t), scale)
    let termY2 := div(mul(gHalf, mul(t, t)), mul(scale, scale))
    let y := sub(termY1_t, termY2)
    
    // Check collision bounds
    if iszero(lt(x, targetX)) {
        if iszero(lt(y, targetY)) {
            hitTarget := 1
        }
    }
    
    // Terrain height check (mocked threshold: y <= 0 is ground)
    if iszero(gt(y, 0)) {
        hitTerrain := 1
    }
}
```

---

## 3. Comparative Chronology

| Aspect | Image Computer Products | Xonox |
| :--- | :--- | :--- |
| **Active Era** | 1979 - early 1980s | 1983 (Dissolved in crash) |
| **Parent Organization** | The Image Producers, Inc. | K-tel Software |
| **Cartridge Strategy** | Standard cassette/cartridge media | "Double-Ender" Reversible dual-PCB cart |
| **Flagship Physics** | Base runner state transitions & hit timing | Wind-drag artillery projectile trajectories |
| **Signature Titles** | *All Star Baseball*, *IMAGE BASIC* | *Artillery Duel*, *Chuck Norris Superkicks* |

---

> [!TIP]
> Xonox's physical "Double-Ender" design is the ultimate analogue equivalent to modern software-level multi-boot registers. In virtual machines and smart contracts, this translates directly to state flags modifying execution routes.
