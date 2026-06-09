# Datasoft Inc. — Technical & Historical Retrospective

Datasoft, Inc., founded in 1980 by Pat Ketchum in Chatsworth, California, was a key driver of the early 8-bit home computer software industry. Developing for Atari 8-bit, Apple II, and Commodore 64 platforms, Datasoft specialized in smooth frame-rate ports, movie licenses, and pioneering action-platformers.

This document details the mechanics and technical systems of their landmark title, ***Bruce Lee*** (1984), and details how its systems map to low-level assembly paradigms and modern on-chain emulators.

---

## 1. Core Innovations of *Bruce Lee* (1984)

Originally programmed by Ron J. Fortier for the Atari 8-bit platform and subsequently ported to the Commodore 64, *Bruce Lee* is recognized as a pioneer of the hybrid platform-fighter (beat-'em-up) genre.

### 1.1 Asymmetrical Cooperative/Competitive Multiplayer
* **The Mechanic:** Player 1 controls Bruce Lee. Player 2 has the unique ability to control the **Green Yamo** (a sumo wrestler enemy). Player 2 can choose to either help Bruce Lee navigate the levels (cooperative) or attack him (competitive).
* **AI Takeover:** If Player 2 is inactive, the game's internal CPU state takes control of Yamo's coordinate changes, seeking out Bruce Lee using a simple state machine.

```
                   +-----------------------------+
                   |     Controller Registry     |
                   +-----------------------------+
                   |  P1 -> Bruce Lee (Active)   |
                   |  P2 -> Green Yamo           |
                   |        ├─ Joystick Active   |
                   |        │  └─ P2 Manual      |
                   |        └─ Joystick Idle     |
                   |           └─ AI State Mach  |
                   +-----------------------------+
```

### 1.2 Flick-Screen Labyrinth Navigation
* **The Mechanic:** The wizard's tower spans 20 distinct single-screen "chambers."
* **Transition Logic:** Triggered when Bruce Lee's coordinate reaches the horizontal screen boundary, instantly reloading the background layout matrices.
* **Objective Lock:** Bruce Lee must collect all lanterns in the active chamber to release the door locks and advance to the adjacent screen.

### 1.3 Collision & Momentum Physics
* **The Flying Kick:** Initiated by moving horizontally and triggering the jump button. It applies forward horizontal momentum and a wider rectangular bounding box, stunning opponents upon collision.
* ** Electrified Hazards:** High-voltage walls and lasers check intersection flags. Touching them resets player coordinates to the starting zone of the active chamber.

---

## 2. On-Chain Emulation Schema

To represent the state of a *Bruce Lee* chamber run on-chain, we map coordinates, inventories, and active entities to namespaced registers:

| Register Address | Register Name | Data Range | Description |
| :---: | :--- | :--- | :--- |
| `54810` | **Active Chamber** | `1 - 20` | Current room ID in the wizard's tower |
| `54811` | **Bruce X** | `0 - 320` | Bruce Lee's horizontal coordinate |
| `54812` | **Bruce Y** | `0 - 200` | Bruce Lee's vertical coordinate |
| `54813` | **Yamo X / Y** | `uint16` | Packed coordinates of Green Yamo |
| `54814` | **Ninja X / Y** | `uint16` | Packed coordinates of the Ninja |
| `54815` | **Remaining Lanterns**| `0 - 15` | Lantern count remaining to open the door |

---

## 3. Chronological Datasoft Milestones

| Year | Title | Genre | Innovation |
| :---: | :--- | :--- | :--- |
| **1982** | *Canyon Climber* | Platformer | Multilayered vertical platform climbing |
| **1984** | *Bruce Lee* | Beat-'em-up / Platformer | Asymmetric P2 takeover and hybrid physics |
| **1984** | *Conan: Hall of Volta* | Action-Platformer | Multi-weapon inventory and boss challenges |
| **1985** | *The Goonies* | Puzzle-Platformer | Multi-character cooperation puzzle layouts |
| **1985** | *Zorro* | Puzzle-Adventure | Layered inventory item-interaction triggers |

---

## 4. Platform Implementation: On-Chain Fighter Physics

To benefit our emulator platform, we have implemented a high-performance **2D Platformer & Fighter Physics Solver** directly inside our compiled Yul graphics system contract:

### updateFighterState
Exposed under selector `0xd6c09b2e` in [graphicsSystem.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/graphicsSystem.yul), this solver handles:
* **Physics Integration:** Integrates velocity vectors (`vx`/`vy`) into character coordinates (`x`/`y`).
* **Active Gravity & Floor Restraints:** Applies gravity decays (`vy := sub(vy, 1)`) and resolves floor boundaries (`y = 0`) to manage jumping and falling.
* **Flick-Screen / Screen Constraints:** Restricts character position within $0 \le x \le 320$.
* **Asymmetrical Hitbox Collision Detection:** Dynamically sets bounding-box sizes depending on active actions (expanding horizontal check widths to `24` during flying kicks vs. standard `16` widths) to verify opponent hits.

### ZMM VM Validation Test
We created and verified the solver using the in-process **ZMM Virtual Machine** to compile and loop-test the physics engine in milliseconds without transaction costs:
```bash
gcc -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -Iinc -Isrc -Isrc/firmware -O3 -g -march=native -flax-vector-conversions tests/test_zmm_fighter.c $(ls obj/*.o | grep -v main.o) obj/firmware/*.o -o tests/test_zmm_fighter -lm -lpthread -ldl -lubsan -rdynamic -lwayland-client -lvulkan -ljpeg && ./tests/test_zmm_fighter
```

---

> [!TIP]
> Datasoft's asymmetrical P2 design and platform engine represents a milestone in **action-adventure cooperative play**, establishing templates that later console-based beat-'em-ups would follow.

