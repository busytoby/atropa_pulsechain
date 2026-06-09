# Atari GTIA Hardware Collision Detection

Unlike the Commodore VIC-II (which uses a single sprite-to-sprite and sprite-to-background mask), the **Atari GTIA (George's Television Interface Adaptor)** features highly granular, dedicated collision registers. 

These registers allow real-time detection of overlap between **Players** (P0–P3), **Missiles** (M0–M3), and **Playfields** (PF0–PF3, representing background colors/graphics) directly in hardware. 

---

## 1. Register Layout

The collision registers are read-only registers located in the GTIA address space. They are cumulative: once a collision is detected by the TV electron beam, the corresponding register bit stays set to `1` until cleared by writing any value to the write-only `HITCLR` register.

### 1.1 Player to Player Collisions (`P0PL` to `P3PL`)
These registers track which player has overlapped with another player.
* **Addresses**: `$D00C` (P0PL) to `$D00F` (P3PL)
* **Bit Representation**:
  * Bit 0: Collision with Player 0
  * Bit 1: Collision with Player 1
  * Bit 2: Collision with Player 2
  * Bit 3: Collision with Player 3

### 1.2 Player to Playfield Collisions (`P0PF` to `P3PF`)
These registers track overlaps between players and background playfield colors/boundaries.
* **Addresses**: `$D004` (P0PF) to `$D007` (P3PF)
* **Bit Representation**:
  * Bit 0: Overlap with Playfield 0 (PF0)
  * Bit 1: Overlap with Playfield 1 (PF1)
  * Bit 2: Overlap with Playfield 2 (PF2)
  * Bit 3: Overlap with Playfield 3 (PF3)

### 1.3 Missile to Player Collisions (`M0PL` to `M3PL`)
These registers track which missile has hit a player.
* **Addresses**: `$D008` (M0PL) to `$D00B` (M3PL)
* **Bit Representation**:
  * Bit 0: Collision with Player 0
  * Bit 1: Collision with Player 1
  * Bit 2: Collision with Player 2
  * Bit 3: Collision with Player 3

### 1.4 Missile to Playfield Collisions (`M0PF` to `M3PF`)
These registers track missile overlaps with the background.
* **Addresses**: `$D000` (M0PF) to `$D003` (M3PF)

---

## 2. Hardware Collision Detection Logic

Below is a pseudo-algorithm modeling the GTIA hardware pipeline checking for overlaps:

```
For each active horizontal scanline:
    For each pixel X on the scanline:
        Let active_elements = Bitmask of rendering elements at pixel X:
            - P0, P1, P2, P3
            - M0, M1, M2, M3
            - PF0, PF1, PF2, PF3
            
        // If Player 0 and Player 1 are both rendering at this pixel:
        If (active_elements contains P0) AND (active_elements contains P1):
            Set bit 1 in P0PL register
            Set bit 0 in P1PL register

        // If Player 2 and Playfield 3 are both rendering at this pixel:
        If (active_elements contains P2) AND (active_elements contains PF3):
            Set bit 3 in P2PF register
```

This pixel-exact overlap is computed on the fly by logic gates in the GTIA as the video signal is generated, bypassing CPU calculations completely.

---

## 3. Simulation Implementation

To demonstrate this system, we have created a simulator script: [test_gtia_collisions.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_gtia_collisions.js).

This script:
1. Simulates GTIA register state in memory.
2. Models 2D rendering coordinates for players, missiles, and playfield shapes.
3. Automatically sets the corresponding bits in `PxPL` and `PxPF` registers upon overlaps.
4. Simulates writing to the `HITCLR` (`$D01E`) register to reset the collision state.
