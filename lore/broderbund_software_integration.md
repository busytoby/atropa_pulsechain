# Broderbund Software Integration Blueprint

To expand the virtual console library of the TSFi2 decentralized OS on Atropa Pulsechain, we will integrate and emulate core game mechanics from legendary Broderbund releases: **Choplifter**, **Lode Runner**, and **Karateka**.

---

## 1. Game Mechanics & Memory Map

### A. Choplifter (Helicopter Rescue Simulation)
Emulates flight mechanics, hostage rescue collision, enemy combat, and landing-pad scoring.
*   **Memory-Mapped Registers**:
    *   `$D6E0` (55008): Player Heli X Coordinate (0-319)
    *   `$D6E1` (55009): Player Heli Y Coordinate (0-199)
    *   `$D6E2` (55010): Helicopter State Flags:
        *   Bit 0: Direction (0 = Left, 1 = Right)
        *   Bit 1-2: Speed/Thrust level (0-3)
    *   `$D6E3` (55011): Fuel Level (0-255)
    *   `$D6E4` (55012): Hostages On Board (0-16)
    *   `$D6E5` (55013): Hostages Rescued (Score)
    *   `$D6E6` (55014): Physics Tick Trigger ($D6E6 = 1 updates Heli coordinates using current velocity and charges fuel tax)

### B. Lode Runner (2D Grid Platformer)
Emulates the 2D tile layout, collection of gold bars, guard tracking AI, and dig mechanics.
*   **Tile Definitions**:
    *   `0x00`: Empty space
    *   `0x01`: Brick Wall (diggable)
    *   `0x02`: Solid block (undiggable)
    *   `0x03`: Ladder
    *   `0x04`: Hand-rail / Rope
    *   `0x05`: Gold Bar
    *   `0x06`: Dug Hole (temporary open space, refills after timer)
*   **Memory-Mapped Registers**:
    *   `$D6EA` (55018): Player X Tile Position (0-39)
    *   `$D6EB` (55019): Player Y Tile Position (0-24)
    *   `$D6EC` (55020): Action Trigger:
        *   `1`: Dig Left (replaces wall tile at X-1, Y+1 with dug hole `0x06`)
        *   `2`: Dig Right (replaces wall tile at X+1, Y+1 with dug hole `0x06`)
        *   `3`: Collect Gold (scans current position for `0x05`, increments score, clears tile)

### C. Karateka (Martial Arts Combat)
Emulates combat states, collision checks for strikes (punch/kick), and health bar adjustments.
*   **Memory-Mapped Registers**:
    *   `$D6F0` (55024): Player Stance Mode:
        *   `0`: Normal / Running (vulnerable to instant defeat if hit)
        *   `1`: Combat / Stance
    *   `$D6F1` (55025): Player Attack Strike:
        *   `0`: Idle
        *   `1`: Punch (short range, fast)
        *   `2`: Kick (long range, slow)
    *   `$D6F2` (55026): Enemy Stance Mode
    *   `$D6F3` (55027): Distance between Player and Enemy (0-255)
    *   `$D6F4` (55028): Player Health (0-100)
    *   `$D6F5` (55029): Enemy Health (0-100)
    *   `$D6F6` (55030): Combat Tick Trigger (calculates hit registration based on distance and active strike)

---

## 2. Implementation & Test Strategy

We will build emulations in the Yul CPU or as test scripts that write to simulated virtual registers to verify the math logic.
*   We'll start by implementing **Choplifter** physics tick mechanics and hostage collection checks on-chain.
*   Then we'll implement **Lode Runner** digging mechanisms with tile replacement checks.
