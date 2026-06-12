# 🎮 TSFi2 Gameplay Mechanics & Retro Emulator Analysis (100% Coverage)

This document provides a technical and architectural analysis of the gameplay simulation, physics models, and state-machine verification models for **all 11 games** implemented in the **TSFi2** framework (via Yul EVM and C-level emulators).

---

## 1. Lode Runner (1983 - Broderbund)
* **Grid Traversal & Scoring**: Maps a $40 \times 24$ text-grid representation of Screen RAM starting at address `1024`. Scoring checks cell contents (e.g., gold bar `0x05`) and transitions it to empty space (`0x00`) while incrementing score registers.
* **Digging & Trap States**: Digging left/right temporarily clears adjacent brick tiles (`0x01` -> `0x00`). If a guard's coordinates intersect a dug-out cell, the guard enters a trapped state, disabling movement.
* **MMIO Registers**: `$D6EA` (Player X), `$D6EB` (Player Y), `$D6EC` (Action: `1`=Dig Left, `2`=Dig Right, `3`=Gold), `$D6ED` (Score).

## 2. Karateka (1984 - Broderbund)
* **Stance States**: The player transitions between **Normal Stance** (used for running) and **Combat Stance** (used for fighting). Running into an enemy in Normal Stance results in instant death.
* **Hitbox & Range Thresholds**: Attacks are verified by range boundaries:
  - **Punch**: Short range (Distance $\le 15$), lower damage ($10$ HP depletion).
  - **Kick**: Long range (Distance $\le 25$), higher damage ($15$ HP depletion).
* **MMIO Registers**: `$D700` (Player Stance), `$D701` (Player Strike), `$D702` (Enemy Stance), `$D703` (Distance), `$D704` (Player HP), `$D705` (Enemy HP), `$D706` (Tick Trigger).

## 3. Choplifter (1982 - Broderbund)
* **Helicopter Physics**: Simulates 2D flight dynamics, calculating vertical acceleration based on thrust, gravity, and pitch-dependent horizontal thrust.
* **Rescue Mechanics**: Bounded cargo limits (16 passengers). Safe landing requires a flat pitch ($0^\circ$) and descending velocity under a critical landing threshold ($V_y \le 5\text{ units/s}$); exceeding this triggers a crash state.
* **MMIO Registers**: `$D710` (Chopper X), `$D711` (Chopper Y/Alt), `$D712` (Pitch), `$D713` (Velocity Y), `$D714` (Cargo), `$D715` (State: `0`=Airborne, `1`=Landed, `2`=Crashed).

## 4. Cannonball Blitz (1982 - Sierra On-Line)
* **Ramp Physics**: Simulates barrel movement down inclined planes. Barrels gain momentum proportionally to the slope angle and change direction upon contacting wall boundaries.
* **Jump and Collision Windows**: Bounding box overlap detection verifies collisions: if a barrel overlaps the player while the player is not in a high jump state (`Jump State == 1`), health is depleted.
* **MMIO Registers**: `$D720` (Player X), `$D721` (Player Y), `$D722` (Jump State), `$D723` (Barrel X), `$D724` (Barrel Y), `$D725` (Ramp Slope), `$D726` (Player HP).

## 5. The Oregon Trail (1975/1985 - MECC)
* **Resource Consumption & Sickness**: Calculates daily food depletion and health decay based on travel speed and rations mode. Low health triggers randomized events like contracting dysentery, draining health daily.
* **River Crossings & Hunting**: Simulates caulking-and-floating versus fording rivers, applying random disaster checks. Hunting consumes ammunition and scales food returns based on active weather states.
* **MMIO Registers**: Implements game state updates using structural C loops tracking distance, food, oxen, and disease status.

## 6. Lunar Lander (1979 - Atari)
* **Descent Physics**: Integrates gravity ($g = -1.62\text{ m/s}^2$ for Moon) and counteracting thruster acceleration:
  $$V_y(t) = V_{y0} + (a_{\text{thrust}} - g) \cdot \Delta t$$
* **Safe Landing Window**: Verifies that descent velocity upon touchdown ($Y \le 0$) does not exceed $V_{\text{safe}} \le 3.0\text{ m/s}$, checking fuel reserves against thrust ticks.
* **Vulkan Render**: Draws vector-graphics terrain meshes and thruster flame particles dynamically.

## 7. Space War (1962 - MIT)
* **Orbital Gravity**: Simulates ship motion under the gravitational pull of a central sun, implementing Keplerian coordinate shifts:
  $$\vec{a} = -G \cdot M \cdot \frac{\vec{r}}{|\vec{r}|^3}$$
* **Boundary Wrap**: Implements wrapping coordinates ($X, Y$) when players cross the screen perimeter.
* **Tactical Resources**: Tracks fuel levels and missile depletion ticks.

## 8. Gauntlet (1985 - Atari Games)
* **Dungeon Traversal**: Integrates tile-based pathfinding checks through maze-like grids, blocking movement through wall boundaries.
* **Health Decay**: Emulates the classic arcade timer mechanic: player health decays by $1\text{ point/second}$ naturally, requiring food pickups to survive.
* **Enemy Spawns**: Manages active ghost/monster spawners and evaluates range-based missile collisions.

## 9. Yars' Revenge (1982 - Atari)
* **Shield Breakdown**: Tracks cell destruction across the Qotile shield barrier matrix during Yar nibble attacks.
* **Zorlon Cannon**: Emulates cannon spawn and firing trajectories, checking alignment with the player.
* **Neutral Zone**: Implements safe zone physics where the player is immune to the Qotile swirl but cannot fire.

## 10. Mystery House (1980 - On-Line Systems)
* **Adventure Parser & Room States**: Implements room-to-room directed graph transitions based on verb-noun commands (e.g. `GO NORTH`, `GET KNIFE`).
* **Inventory Management**: Tracks item ownership and room presence tags (e.g., verifying that a key is in the inventory before unlocking a door).

## 11. Fighter (ZMM Fighter)
* **Combat Stance & Hits**: Models fighter mechanics including distance-based attacks, block reflexes, health depletion, and basic AI stances.
* **MMIO State**: Verifies fast register transitions for frame-by-frame character combat ticks.

---

## Comparative Game Metrics Summary

| Game | State representation | Hazard Logic | Core Math Model |
| :--- | :--- | :--- | :--- |
| **Lode Runner** | $40\times24$ RAM grid | Guard adjacency | Discrete Coordinate Check |
| **Karateka** | Stance State Machine | Combat stance vs normal | Bounding Box range check |
| **Choplifter** | Continuous velocity | Crash speed threshold | Vector acceleration |
| **Cannonball Blitz**| Slope incline coordinate | Bounding box overlap | Gravity & Ramp physics |
| **Oregon Trail** | Resource list | Sickness & Starvation | Sickness probability |
| **Lunar Lander** | Vector position | Descent velocity limit | 1D Gravitational integration |
| **Space War** | Vector position | Sun impact / collision | 2D Gravitational vector field |
| **Gauntlet** | Grid array | Health timer decay | Distance-based collision |
| **Yars' Revenge** | Destructible grid matrix | Swirl collision | Matrix cell state checks |
| **Mystery House** | Room Graph | Incorrect movement | Directed State Graph |
| **Fighter** | Stance State | HP depletion | Normal vs Special hitboxes |

---

> [!IMPORTANT]
> By mapping all 11 retro games to memory-mapped Yul registers and coordinate arrays, TSFi2 ensures robust simulation timing, compile safety, and zero-leak memory verification during active emulation playtests.
