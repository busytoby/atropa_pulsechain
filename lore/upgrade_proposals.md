# Folklore CPU Expansion & Game Upgrade Proposal

By transitioning to the EVM-native **Folklore CPU** in Yul, we are no longer limited to 6502 CPU boundaries. Here are the proposed features to take the "Teddy Bear Rescue" game to the next level:

---

## 1. Native Double Jump Physics (In Yul)
- **Feature**: Enable a floaty double-jump mechanic.
- **Implementation**: Track double-jump ticks inside a Yul storage slot (e.g., `0x42`). When a jump trigger is received while `jumping == 1`, provide a secondary upward impulse (`pvy := -11000`) and cap the jump count to 2.
- **Benefit**: Greatly improves platforming agility, allowing players to easily bypass tight clusters of spikes and bats.

## 2. Dynamic Falling Stalactites (Level 2 Cavern Hazards)
- **Feature**: Randomly falling stalactites triggered by proximity.
- **Implementation**: In Yul, track player proximity to ceiling stalactites. If player $x$ is near a stalactite slot, initiate a falling velocity update. The stalactite accelerates downwards (`y := add(y, vy)`) and checks for collision.
- **Benefit**: Makes the cave level feel active, alive, and challenging without feeling unfair.

## 3. Gargamel Chase AI (Level 3 Boss Logic)
- **Feature**: Dynamic state-based behavior for Gargamel.
- **Implementation**: In Yul, compute the absolute distance between the player and Gargamel:
  ```solidity
  let distance := abs(sub(player_x, gargamel_x))
  ```
  If distance is less than 200, transition Gargamel from "Patrol Mode" to "Chase Mode", increasing his velocity and directing it towards the player's current coordinate.
- **Benefit**: Replaces the static left-to-right patrol with a responsive boss encounter.

## 4. Integrated Trauma Engine (Move logic from C to Yul)
- **Feature**: Keep all gameplay state authority in the virtual machine.
- **Implementation**: Move proximity fear calculations, control reversal (Terrified), and freezing logic (Panicked) inside Yul's physics loop. The Yul contract directly updates slot states for rendering.
- **Benefit**: The C code remains a clean view-controller, while the Yul contract remains the single source of truth for the game state.
