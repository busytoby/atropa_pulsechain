# Integrating Player-Missile Graphics (PMG) with Gauntlet

This document specifies the integration of our virtual **Atari-style Player-Missile Graphics (PMG)** engine (implemented in [pmgSystem.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/pmgSystem.yul)) with the **Gauntlet** game loop, leveraging hardware-level sprites, missiles, and collision registers.

---

## 1. Mappings: Gauntlet Entities to PMG Registers

In the Atari architecture, Gauntlet's characters, projectiles, and hazards are mapped to the hardware's 4 Players and 4 Missiles:

```
[Player 0] -> Thor (Warrior)     [Missile 0] -> Warrior's Throwing Axe
[Player 1] -> Thyra (Valkyrie)   [Missile 1] -> Valkyrie's Sword Projectile
[Player 2] -> Merlin (Wizard)     [Missile 2] -> Wizard's Magic Fireball
[Player 3] -> Questor (Elf)       [Missile 3] -> Elf's Arrow
```

* **Colors (`COLPM0`–`COLPM3`)**: Swapped dynamically to match each character's signature palette (Thor = Blue, Thyra = Red, Merlin = Yellow/White, Questor = Green).
* **Sizes (`SIZEP0`–`SIZEP3`)**: Set to 2x or 4x scale multipliers depending on state transitions (e.g. pulse indicators during combat or when getting hurt).

---

## 2. Projectiles as Missiles

In Gauntlet, players fire constant streams of projectiles. Using standard bitmap sprites for every axe or arrow strains the CPU. PMG solves this by mapping projectiles to **Missiles**:
* **Memory Efficiency**: Missiles are 2-bit wide memory slices, perfect for slim projectiles (arrows, swords).
* **High-Speed Rendering**: Instead of rendering pixel buffers, the CPU updates horizontal position registers (`HPOSM0`–`HPOSM3`) directly.
* **Vertical Movement**: Handled via rapid shift loops in the VBI (Vertical Blank Interrupt), modifying the missile bitmask.

---

## 3. Hardware Collision Detection in Yul

Instead of checking distances between every player and enemy on-chain using costly $O(N^2)$ distance formulas, Gauntlet offloads collision checks to the GTIA emulator registers:

```yul
// resolveGauntletCollisions() -> returns collision mask
let collisionMask := staticcall(gas(), pmgAddress, 0xd4bc89f1, 0, 0, 0)
```

### Collision Mapping
* **Player-to-Player (`P0PL`–`P3PL`)**: Detects when heroes collide or block each other's path.
* **Player-to-Playfield (`P0PF`–`P3PF`)**: Checks if a hero has collided with solid maze walls (Playfield colors) or enemy spawner gates.
* **Missile-to-Playfield (`M0PF`–`M3PF`)**: Instantly detects when a player's axe or arrow hits a wall (destructible wall blocks) or strikes an enemy ghost spawn point.

---

## 4. Double Buffering & VBI Rendering

To prevent visual flickering, Gauntlet implements page-flipping during the Vertical Blank Interrupt (VBI):
* **Page Base Register (`PMBASE`)**: Swapped between alternate buffer pages (`0x4000` and `0x6000`).
* **Frame Sync**: Player and Missile graphics frames are written to the inactive page, then flipped during vertical blanking, guaranteeing artifact-free 60 FPS animation.

---

## 5. EDO-22 Acoustic Space Audio Synthesis

To support retro chiptune sound synthesis during projectile impact events, audio frequencies are calculated dynamically using **EDO-22** (Equal Division of the Octave) formulas:
$$F = 220 \times 2^{\frac{d \pmod{22}}{22}}$$
Where $d$ is the relative pixel distance offset between the projectile missile position and the target spawner/enemy Playfield element. This ensures cryptographically verifiable acoustic feedback loops.

---

## 6. PMG Collision Flash & Damage Effects

When player-to-playfield/enemy collisions occur on-cycle, visual registers update the player's vector envelope:
* **`player.flashFrames`**: Set to `10` frames upon receiving damage.
* **Color Modulation**: Modulates the vector diamond border stroke from `var(--warrior-blue)` to the GTIA red alert color (`#ff1744`) until the counter reaches zero.

---

## 7. Immutable Ledger State Logging

State snapshots containing the level index, score, health values, coordinates, and collision register states are hashed and logged to the immutable ledger partition:
* **Ledger Validation**: Historical blocks are chained using parent-child hash verification links ($Hash = H(Index + PrevHash + State)$).
* **Tamper Prevention**: Any retroactive modifications to previous game frames instantly break the ledger validation chain.

---

## 8. Simulated Web3 / Demo Mode

For offline testing without a local **Auncient** RPC devnet:
* **Local Provider Mocking**: Bypasses connection errors by creating a virtual signer, allowing game coordinates to be read/written smoothly without DOM reflow stutters.
* **Variable Alignment**: Uses `"SIMULATED_WALLET"` as the address marker to disable grid-snapping, ensuring 60fps client-side keyboard updates.
