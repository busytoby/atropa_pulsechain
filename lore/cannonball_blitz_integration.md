# Cannonball Blitz Physics & Collision Engine

This document details the virtual hardware, registers, and Yul operating platform implementation for **Cannonball Blitz** (the classic 1982 platformer clone) on the TSFi2 console. It maps player physics, scaffolding collisions, rolling cannonballs, and game boundaries to on-chain registers under a unified **Diyat Arcade Tax**.

---

## 1. Emulated Cannonball Blitz Register Map ($D680–$D69F)

The Cannonball Blitz engine registers reside at `$D680`–`$D691` (54912–54929):

| Register Address (Hex) | Register Address (Dec) | Access Mode | Description |
| :--- | :--- | :--- | :--- |
| `$D680` | `54912` | Read / Write | **Player X Coordinate**: Character horizontal position (0–39). |
| `$D681` | `54913` | Read / Write | **Player Y Coordinate**: Character vertical position (0–24). |
| `$D682` | `54914` | Read / Write | **Player X Velocity**: Horizontal movement speed (-1, 0, 1). |
| `$D683` | `54915` | Read / Write | **Player Y Velocity**: Vertical climb or jump speed (-1, 0, 1). |
| `$D684`–`$D685` | `54916`–`54917` | Read / Write | **Cannonball 1 X/Y**: First obstacle coordinate. |
| `$D686` | `54918` | Read / Write | **Cannonball 1 Active**: Boolean flag enabling/disabling the obstacle. |
| `$D68D` | `54925` | Read-Only | **Game Score**: Current level score accumulator. |
| `$D68F` | `54927` | Write-Only | **Physics Step Strobe**: Writing `1` executes a single frame step (updates movements and checks collisions). |
| `$D690` | `54928` | Read-Only | **Collision Register**: Set to `1` if player collides with a cannonball. |
| `$D691` | `54929` | Write-Only | **Player Action**: Writing `1` triggers a jump, `2` climbs ladder. |

---

## 2. On-Chain Physics & Collision Routine (Yul)

When the Physics Step Strobe (`$D68F`) is activated, the Yul engine:

1. **Applies Gravity & Scaffolding Collisions**:
   * Scaffolds are mapped to rows 8, 16, and 24.
   * If the player is not on a scaffold, gravity pulls them down (`Player Y` increases).
2. **Updates Rolling Cannonballs**:
   * Rolling obstacles traverse along the scaffolds, dropping and reversing direction at edge boundaries.
3. **Detects Player Collisions**:
   * If `Player X == Obstacle X` and `Player Y == Obstacle Y`, the game sets the Collision Register (`$D690`) to `1` and resets player position.
4. **Arcade Tax and Event Logging**:
   * Charges 1 OTRT arcade play tax.
   * Emits the event log:
     ```solidity
     event CannonballStep(address indexed player, uint256 score, bool collided);
     ```
     Topic: `0xbc8ad72f2ba16a9205578bc41b22e18413b63200ff9dfe8bc9c8be7ce6823908`.
