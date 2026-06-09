# Synapse Software — Technical & Physics Analysis

Founded in 1981 by **Ihor Wolosenko** and **Ken Grant**, **Synapse Software** was a premier developer of high-performance action titles for Atari 8-bit computers and the Commodore 64. Rather than focusing solely on arcade clones, Synapse became famous for pushing technical limits, implementing innovative physics models, and hiring legendary programmers like **Steve Hales** and **William Mataga**.

This document details Synapse's key mechanical milestones, specifically ***Fort Apocalypse*** (1982) and ***Shamus*** (1982), demonstrating how their 2D flight and state architectures translate to low-level assembly.

---

## 1. Flagship Technical Innovations

### 1.1 Helicopter Aerodynamics in *Fort Apocalypse* (1982)
Created by Steve Hales, *Fort Apocalypse* features a multidirectional scrolling shooter where the player pilots a Rocket Copter:
* **The Flight Model:** The game implements gravity and inertia vectors:
  - Vertical acceleration combines gravity ($g$) and variable rotor lift ($F_{\text{lift}}$):
    $$a_y = F_{\text{lift}} - g$$
  - Horizontal movement is governed by tilt-based thrust vectors:
    $$a_x = F_{\text{lift}} \cdot \sin(\theta)$$
* **Landing Pad Detection:** To rescue hostages, the player must land flat on designated pads. Landing requires the helicopter to align its landing gear (yaw angle $\approx 0$) and touch down below a maximum safe downward velocity threshold ($v_{\text{safe}}$) to prevent crashing.

### 1.2 Maze State-Tracking in *Shamus* (1982)
Developed by William Mataga, *Shamus* is a maze action game featuring room-to-room navigation:
* **Color-Coded Key States:** Progression is gated by locks of four distinct colors (Blue, Green, Yellow, Red). 
* **State Machine representation:** The program tracks the player's keys using a simple 4-bit register. Door openings query this register and clear the corresponding bit upon access.

---

## 2. On-Chain Emulation Schema: Helicopter Landing Physics

To model Steve Hales' flight and landing logic in Yul, we can evaluate coordinates, velocities, and orientation angles upon touchdown.

### 2.1 Physics Registers
| Memory Slot | Register Name | Data Range | Description |
| :---: | :--- | :--- | :--- |
| `55400` | **Velocity X** | `-127 to 127` | Horizontal speed register |
| `55401` | **Velocity Y** | `-127 to 127` | Vertical speed register (negative is downward) |
| `55402` | **Helicopter Yaw** | `0 - 359` | Flight tilt angle (degrees, 0 is upright) |
| `55403` | **Pad Center X** | `0 - 320` | Landing pad horizontal center |
| `55404` | **Pad Top Y** | `0 - 200` | Landing pad vertical top boundary |

### 2.2 Touchdown Collision Validator in Yul
The following Yul function determines whether the copter successfully landed on the pad or crashed based on velocity thresholds, coordinates, and yaw offsets:

```yul
// Validates helicopter touchdown physics on landing pad
function checkLandingTouchdown(copX, copY, copYaw, vx, vy, padX, padY) -> landingState {
    // landingState outcomes:
    // 0 = In flight
    // 1 = Successful Landing (Safe state)
    // 2 = Crash (Velocity too high or tilted orientation)

    // Check if helicopter base is at pad height (with tolerance of 2 units)
    let isAtPadY := 0
    if and(iszero(lt(copY, padY)), lt(sub(copY, padY), 3)) {
        isAtPadY := 1
    }

    if isAtPadY {
        // 1. Position Alignment Check
        // Copter center must align with pad center (width 32, i.e., distance < 16)
        let xDiff := 0
        if gt(copX, padX) { xDiff := sub(copX, padX) }
        if iszero(gt(copX, padX)) { xDiff := sub(padX, copX) }
        
        let aligned := lt(xDiff, 16)

        // 2. Orientation (Yaw) Alignment Check
        // Copter must be upright (yaw offset < 10 degrees from 0/360)
        let upright := 0
        if or(lt(copYaw, 10), gt(copYaw, 350)) {
            upright := 1
        }

        // 3. Speed Check
        // Downward vertical velocity (vy) must not exceed safe threshold (e.g. -4)
        // Horizontal velocity (vx) must be negligible (absolute value < 3)
        let absVx := vx
        if lt(vx, 0) { absVx := sub(0, vx) }
        
        // vy is negative for downward flight; vertical speed must be between -4 and 0
        let slowLanding := 0
        // checks if vy >= -4 (in two's complement, greater than or equal is iszero(lt(vy, -4)))
        let vySafe := iszero(lt(vy, sub(0, 4)))
        if and(vySafe, lt(absVx, 3)) {
            slowLanding := 1
        }

        // Evaluate conditions
        if and(aligned, and(upright, slowLanding)) {
            landingState := 1 // Safe Landing
        }
        if iszero(and(aligned, and(upright, slowLanding))) {
            landingState := 2 // Crash
        }
    }
}
```

---

## 3. Synapse Electronic Novels & The BTZ Parser

In 1984–1985, Synapse Software diversified from action arcade games into highly literary interactive fiction, coining the term **"Electronic Novels."**

### 3.1 The BTZ (Better Than Zork) Engine
Written primarily by **William Mataga** and **Steve Hales**, the **BTZ engine** was designed to surpass Infocom's Z-machine parser capabilities:
*   **Sentence Complexity**: It supported compound sentences, nested clauses, and multiple direct objects (e.g., `TAKE ALL BUT THE BLUE KEY AND THEN PUT THEM IN THE BOX`).
*   **Dynamic NPCs**: Characters in BTZ games possessed distinct schedules, moving independently through rooms and reacting dynamically to the player's conversation inputs rather than waiting for triggers.
*   **Memory Paging**: Because C64 and Atari 8-bit RAM was limited to 64 KiB, the BTZ engine utilized an advanced virtual disk paging schema to pull text descriptions and vocabulary trees from floppy disk tracks on-demand.

### 3.2 Key Interactive Fiction Titles
*   ***Mindwheel* (1985)**: Written by U.S. Poet Laureate Robert Pinsky. The player travels through the minds of deceased historical figures (a poet, a musician, a dictator) to retrieve the "Wheel of Wisdom."
*   ***Brimstone* (1985)**: Based on Gawain’s dream journey, written by novelist James Paul.
*   ***Essex* (1985)**: A sci-fi space opera written by Bill Darrah.
*   ***Breakers* (1985)**: A dystopian cyberpunk story written by Rod Smith.

---

## 4. Comparative Chronology

| Aspect | Muse Software | Synapse Software |
| :--- | :--- | :--- |
| **Active Era** | 1978 - 1987 | 1981 - 1985 (acquired by Broderbund) |
| **Key Founders** | Ed Zaron, Silas Warner | Ihor Wolosenko, Ken Grant |
| **Primary Platforms** | Apple II, Commodore 64 | Atari 8-bit, Commodore 64 |
| **Simulation Core** | Audio synthesizer & compilers | Dynamic 2D kinematics, map scrolling, & BTZ parser |
| **Signature Titles** | *Castle Wolfenstein*, *RobotWar* | *Fort Apocalypse*, *Shamus*, *Mindwheel* |

---

> [!TIP]
> Steve Hales' *Fort Apocalypse* implemented variable vector mechanics on an 8-bit 6502 CPU by caching local vector differences inside the zero page, freeing registers to update scrolling maps at high speeds.

