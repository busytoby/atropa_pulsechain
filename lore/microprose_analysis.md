# MicroProse Software — Technical & Simulation Analysis

Founded in 1982 by systems analyst **Sid Meier** and military pilot **Bill Stealey**, **MicroProse Software** was a dominant force in the 8-bit and 16-bit personal computer eras. Operating primarily on the Commodore 64 and Atari 8-bit families, MicroProse pioneered military and civilian vehicle simulations. Sid Meier’s core design philosophy—that "a game is a series of interesting decisions"—led the company to prioritize playable goal-oriented simulation structures over pure dry mathematical modeling.

This document details the mechanics of Sid Meier’s landmarks, specifically ***Solo Flight*** (1983), and demonstrates how instrument-based navigation systems map to low-level execution logic.

---

## 1. Flagship Mechanical Innovations

### 1.1 Solo Flight (1983) & Objective-Based Simulation
Prior to *Solo Flight*, flight simulators were largely aimless sandboxes focusing exclusively on flight stability. Sid Meier shifted this paradigm:
* **The Mail Pilot Game Loop:** Players were tasked with transporting mail cargo between multiple airports, forcing them to balance fuel consumption, cargo weight (which modified the aircraft's lift/drag vectors), and schedules.
* **Environmental Turbulence:** Meier modeled crosswinds, stalls, engine overheating emergencies, and visibility degradation (forcing instrument-only flights).

### 1.2 VOR (Very High Frequency Omnidirectional Range) Navigation
In the era before GPS, *Solo Flight* was one of the first home computer games to implement **VOR Navigation**:
* **The Mechanic:** VOR ground stations emit VHF radio signals. The aircraft's VOR receiver determines the bearing (radial) from the station to the aircraft.
* **Instrument Display:** The instrument panel features an OBI (Omni Bearing Indicator) dial. The needle shifts left or right depending on whether the plane is off-course relative to the selected radial (radial deviation).
* **The Geometry:** The bearing angle ($\theta$) is calculated relative to the station coordinate $(X_s, Y_s)$ and plane coordinate $(X_p, Y_p)$:
  $$\theta = \arctan2(Y_s - Y_p, X_s - X_p)$$

---

## 2. On-Chain Emulation Schema: VOR Radial Deviation

To simulate VOR navigation on-chain, we calculate the bearing angle from a plane to a transmitter and derive the radial alignment error.

### 2.1 Navigation Registers
| Memory Slot | Register Name | Data Range | Description |
| :---: | :--- | :--- | :--- |
| `55200` | **Plane Coordinate X** | `0 - 65535` | Aircraft X position (fixed-point) |
| `55201` | **Plane Coordinate Y** | `0 - 65535` | Aircraft Y position (fixed-point) |
| `55202` | **VOR Station X** | `0 - 65535` | Ground VOR transmitter X coordinate |
| `55203` | **VOR Station Y** | `0 - 65535` | Ground VOR transmitter Y coordinate |
| `55204` | **Target Radial** | `0 - 359` | Selected VOR course radial (degrees) |
| `55205` | **Deviation Needle** | `-10 to +10` | Needle position on OBI dial |

### 2.2 VOR Bearing and Deviation Resolver in Yul
The following Yul function computes the absolute bearing angle from the plane to the VOR station using coordinate trigonometry and outputs the CDI (Course Deviation Indicator) needle shift:

```yul
// Calculates bearing angle and course deviation for VOR instrumentation
function resolveVOR(px, py, sx, sy, targetRadial) -> bearing, deviation {
    // 1. Calculate coordinate differences
    let dx := 0
    let dy := 0
    let isNegX := lt(sx, px)
    let isNegY := lt(sy, py)
    
    if isNegX { dx := sub(px, sx) }
    if iszero(isNegX) { dx := sub(sx, px) }
    if isNegY { dy := sub(py, sy) }
    if iszero(isNegY) { dy := sub(sy, py) }

    // 2. Approximated arctan2 for bearing angle (0 - 360)
    // Avoid division-by-zero
    if iszero(dx) { dx := 1 }
    
    // ratio = (dy * 1000) / dx
    let ratio := div(mul(dy, 1000), dx)
    
    // Linearized arctan approximation for bearing direction
    let angle := 0
    if lt(ratio, 268) { angle := div(mul(ratio, 15), 268) } // 0 - 15 degrees
    if and(iszero(lt(ratio, 268)), lt(ratio, 577)) { angle := add(15, div(mul(sub(ratio, 268), 15), 309)) } // 15 - 30 degrees
    if and(iszero(lt(ratio, 577)), lt(ratio, 1000)) { angle := add(30, div(mul(sub(ratio, 577), 15), 423)) } // 30 - 45 degrees
    if iszero(lt(ratio, 1000)) { angle := add(45, div(mul(sub(ratio, 1000), 45), 10000)) } // 45 - 90 degrees
    if gt(angle, 90) { angle := 90 }

    // Map quadrant based on signs
    if iszero(isNegX) {
        if iszero(isNegY) { bearing := angle } // Quadrant 1 (0-90)
        if isNegY { bearing := sub(360, angle) } // Quadrant 4 (270-360)
    }
    if isNegX {
        if iszero(isNegY) { bearing := sub(180, angle) } // Quadrant 2 (90-180)
        if isNegY { bearing := add(180, angle) } // Quadrant 3 (180-270)
    }

    // 3. Compute course deviation: error = bearing - targetRadial
    let error := 0
    let isNegErr := lt(bearing, targetRadial)
    if isNegErr { error := sub(targetRadial, bearing) }
    if iszero(isNegErr) { error := sub(bearing, targetRadial) }
    
    // Normalize angular difference (e.g. 359 vs 0 is 1 deg error)
    if gt(error, 180) { error := sub(360, error) }
    
    // Needle deflection (scale of -10 to +10, clamped at 10 degrees error)
    deviation := error
    if gt(deviation, 10) { deviation := 10 }
    
    // Apply sign to deflection needle
    if isNegErr {
        deviation := sub(0, deviation)
    }
}
```

---

## 3. Comparative Chronology

| Aspect | Quicksilva, Inc. | MicroProse Software |
| :--- | :--- | :--- |
| **Active Era** | 1980 - 1990 | 1982 - 2003 (brand revived later) |
| **Co-Founders** | Nick Lambert | Sid Meier, Bill Stealey |
| **Engineering Core** | Real-time isometric occlusions | Navigation instrument modeling (VOR/dials) |
| **Flagship Focus** | Painter's Algorithm rendering | Playable aerodynamic & submarine physics |
| **Key Titles** | *Ant Attack*, *Bugaboo (The Flea)* | *Solo Flight*, *F-15 Strike Eagle*, *Silent Service* |

---

> [!TIP]
> Sid Meier's decision to use discrete linear ratio ranges to approximate VOR angles represents the essence of 8-bit optimization: bypassing expensive trigonometric math libraries in favor of high-speed integer approximations.
