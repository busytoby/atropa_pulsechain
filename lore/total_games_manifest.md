# 📚 TSFi2 Complete Games & Simulations Manifest

This document indexes, counts, and provides minimal documentation for **all 46 games and interactive simulations** configured across our native C/AVX-512 testing pipeline, our DApp browser dashboard, and our emulated DATAMOST Software to Go library.

---

## 1. ⚡ Native C/AVX-512 Pipeline (14 Games)
*These games are integrated directly into our C testing runtime, compile via the `Makefile`, and verify core physical/logic states using EVM Yul emulation with zero-leak assertions.*

### 1.1 🏃‍♂️ Lode Runner
* **Description**: Retro puzzle-platformer where players collect gold and dig temporary traps in brick walls.
* **Emulation Hook**: Maps a $40 \times 24$ Screen RAM grid at address `1024` with MMIO registers at `$D6EA - `$D6ED`.
* **Source**: [test_lode_runner.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_lode_runner.c)

### 1.2 🥋 Karateka
* **Description**: Side-scrolling martial arts action game modeling Combat/Normal stances and range-based hitboxes.
* **Emulation Hook**: Simulates stance check triggers and kick/punch range calculations at `$D700 - `$D706`.
* **Source**: [test_karateka.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_karateka.c)

### 1.3 🚁 Choplifter
* **Description**: Combat helicopter rescue simulator implementing continuous 2D flight dynamics and cargo constraints.
* **Emulation Hook**: Verifies velocity landing limits and passenger count at `$D710 - `$D715`.
* **Source**: [test_choplifter.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_choplifter.c)

### 1.4 💣 Cannonball Blitz
* **Description**: Ladder-climbing action game simulating barrel movement down sloped ramps.
* **Emulation Hook**: Bounding box overlap collision checks are mapped at `$D720 - `$D726`.
* **Source**: [test_cannonball.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_cannonball.c)

### 1.5 🐂 The Oregon Trail
* **Description**: Trail resource management simulation covering food consumption, weather, and river crossing hazards.
* **Emulation Hook**: Tracks distance, food, and health metrics over time.
* **Source**: [test_oregon_trail.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_oregon_trail.c)

### 1.6 🧑‍🚀 Lunar Lander
* **Description**: Physics-based spacecraft landing simulator incorporating thruster acceleration and moon gravity.
* **Emulation Hook**: Verifies touchdown speed ($V_y \le 3.0\text{ m/s}$) and fuel consumption.
* **Source**: [test_wayland_lunar_lander.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_lunar_lander.c)

### 1.7 🌌 Space War
* **Description**: Keplerian orbital simulation featuring ships orbiting a central gravity-well.
* **Emulation Hook**: Calculates gravitational pull and handles wrapping coordinates at screen borders.
* **Source**: [test_wayland_space_war.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_space_war.c)

### 1.8 🛡️ Gauntlet
* **Description**: Grid-based dungeon crawler implementing naturally decaying health.
* **Emulation Hook**: Health decreases by 1 point per second to simulate the arcade coin-operation timer.
* **Source**: [test_wayland_gauntlet.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_gauntlet.c)

### 1.9 👾 Yars' Revenge
* **Description**: Classic shooter modeling destructible grid shield matrices and Zorlon Cannon trajectories.
* **Emulation Hook**: Evaluates shield nibble states and neutral zone immunities.
* **Source**: [test_wayland_yars_revenge.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_yars_revenge.c)

### 1.10 🏚️ Mystery House
* **Description**: Text-based adventure game utilizing verb-noun parsers and room-transition graphs.
* **Emulation Hook**: Monitors room connectivity and inventory possession.
* **Source**: [test_zmm_mystery_house.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_zmm_mystery_house.c)

### 1.11 🥊 Fighter (ZMM Fighter)
* **Description**: Simple 1v1 combat simulator checking hits, blocks, and health metrics.
* **Emulation Hook**: Tracks basic registers for character states.
* **Source**: [test_zmm_fighter.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_zmm_fighter.c)

### 1.12 🍄 Smurfs Rescue Manifold
* **Description**: Simulation of C64 *Smurfs: Rescue in Gargamel's Castle* implementing pathing mechanics, jumping over obstacles, and audio triggers.
* **Emulation Hook**: Intercepts jumps, collisions, and gameplay wins.
* **Source**: [test_wayland_smurfs.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_smurfs.c)

### 1.13 ⚾ Hardball Stadium
* **Description**: Native simulation of baseball innings, pitcher state changes, and batting hits/strikes.
* **Emulation Hook**: Verifies visual status outputs and pitch results.
* **Source**: [test_wayland_hardball.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_hardball.c)

### 1.14 🧸 3D Teddy Bear Tournament Visualizer
* **Description**: Visual system check mapping 3D coordinate matrices and movement sequences.
* **Emulation Hook**: Computes position transformations and frame loop updates.
* **Source**: [test_wayland_teddy.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_teddy.c) / [test_wayland_teddy_standalone.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_teddy_standalone.c)

---

## 2. 💻 DApp Browser Dashboard Standalone Frontend Applications (10 Games & Studios)
*These interactive frontends are written in HTML5/JavaScript with rich custom CSS styling, Web Audio, and Canvas rendering.*

### 2.1 🐉 Dragon's Lair (`dragons_lair.html`)
* **Description**: Interactive storybook and ambient synthesizer incorporating analog dither noise and wave shaping.
* **Link**: [dragons_lair.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/dragons_lair.html)

### 2.2 🏰 Star Castle (`star_castle.html`)
* **Description**: Vector-graphics style space shooter featuring concentric rotating shield rings and central target destruction.
* **Link**: [star_castle.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/star_castle.html)

### 2.3 🛡️ Gauntlet Console (`gauntlet.html`)
* **Description**: Browser-based implementation of the multi-directional grid maze shooter.
* **Link**: [gauntlet.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/gauntlet.html)

### 2.4 📜 Z-Machine Interpreter (`zmachine.html`)
* **Description**: Virtual machine for text adventures, running classic games like *Zork* dynamically on-chain.
* **Link**: [zmachine.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/zmachine.html)

### 2.5 📟 RCA COSMAC VIP / ELF (`cosmac_elf.html`)
* **Description**: Emulates the 1802 microprocessor and runs CHIP-8 software (including *Pong*, *Blinky*, and *Space Invaders*).
* **Link**: [cosmac_elf.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/cosmac_elf.html)

### 2.6 ⚾ Hardball Stadium (`hardball_stadium.html`)
* **Description**: Interactive baseball simulator tracking match states, pitches, swings, and stadium graphics.
* **Link**: [hardball_stadium.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/hardball_stadium.html)

### 2.7 🧸 3D Bear Tournament (`teddy_bear_tournament_3d.html`)
* **Description**: Physics-driven 3D tournament where players test genomes of generated 3D teddy bears.
* **Link**: [teddy_bear_tournament_3d.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/teddy_bear_tournament_3d.html)

### 2.8 🎨 Sally Larsen C64 Sprite Console (`sally_larsen_c64.html`)
* **Description**: Sprite animator and retro target shooter showcasing 8-bit graphics configurations.
* **Link**: [sally_larsen_c64.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/sally_larsen_c64.html)

### 2.9 🧬 DNA Breeder Arena (`dna_breeder.html`)
* **Description**: Genetic crossing and breeding environment used to optimize genome fitness criteria.
* **Link**: [dna_breeder.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/dna_breeder.html)

### 2.10 📈 Vaesen Biorhythms (`biorhythm.html`)
* **Description**: Interactive physiological cycle tracker mapping intellectual, emotional, and physical cycles.
* **Link**: [biorhythm.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/biorhythm.html)

---

## 3. 💾 DATAMOST Software to Go Emulated Library (22 Games)
*Interactive software emulations downloadable under a virtual Diyat Library Tax to an emulated floppy drive context inside the DATAMOST dashboard ([datamost.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/datamost.html)).*

### 3.1 🏓 Pong-on-Chain
* **Description**: Two-player paddle collision physics demo simulating simple on-chain game loops.

### 3.2 🐜 Ant Eater
* **Description**: Maze game where the player controls an anteater digging tunnels to eat ants while avoiding predators.

### 3.3 🐸 Princess and the Frog
* **Description**: Frogger-like crossing game verifying pathing and collision bounding boxes.

### 3.4 🦖 Crush, Crumble & Chomp!
* **Description**: Giant monster simulation movie creator where monsters destroy metropolitan centers.

### 3.5 🕷️ Apple Cider Spider
* **Description**: Platformer where a spider navigates factory machinery obstacles.

### 3.6 ⚔️ Ultima-on-Chain
* **Description**: Retro tilemap-driven RPG simulating dungeon-crawling, exploration, and combat.

### 3.7 ⛏️ Miner 2049er
* **Description**: Platformer where Miner Willy claims sectors of a radioactive mine.

### 3.8 🦖 Dino Eggs
* **Description**: Time-traveling platform game to rescue dinosaur eggs before extinction events.

### 3.9 ✈️ Blue Max
* **Description**: WW1 diagonal scrolling flight combat game simulating bombing targets and dogfights.

### 3.10 🕵️ Impossible Mission
* **Description**: Action puzzle agent simulator dodging security robots to locate puzzle pieces.

### 3.11 ⭐ Name That Star
* **Description**: Retro astronomy quiz and constellation mapper.

### 3.12 🚀 Thrust
* **Description**: Gravitational physics flight simulation where players navigate narrow tunnels to retrieve pods.

### 3.13 🥷 The Last Ninja (Ahoy! Issue 33)
* **Description**: Isometric action adventure simulation showcasing combat mechanics.

### 3.14 🥷 The Last Ninja (System 3 1987)
* **Description**: Standard release simulation of the iconic isometric ninja combat game.

### 3.15 🏰 Vault of Terror
* **Description**: Grid-based horror dungeon crawler checking puzzle lock transitions.

### 3.16 🐭 Mouse in the House
* **Description**: Action puzzle tracking food gathering paths.

### 3.17 🔓 Jail Break
* **Description**: Action game checking escape routes, key retrieval, and prison guard patterns.

### 3.18 🐢 Turtle Rescue
* **Description**: Obstacle dodging platformer where turtles are guided to safety.

### 3.19 🗺️ The Adventurer
* **Description**: Early interactive parser-driven text adventure game.

### 3.20 ⚔️ Duel
* **Description**: Turn-based combat simulator testing hitboxes and weapon selection.

### 3.21 👁️ Sentinel
* **Description**: Height-based stealth puzzle simulating energy absorption mechanics.

### 3.22 ⚾ Hardball!
* **Description**: Classic Accolade baseball simulator verifying stats and pitch states.

---

> [!NOTE]
> This totals **46 games and interactive environments** configured and running inside the TSFi2 ecosystem.
