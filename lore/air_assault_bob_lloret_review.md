# Bob Lloret's "Air Assault" — Historical & Technical Review

First published in **Ahoy! Magazine Issue 5 (May 1984, page 45)**, *Air Assault* is an action shoot-'em-up game designed by **Bob Lloret**. The game places the player in command of a ground defense cannon, fighting off waves of descending enemy aircraft.

We have fully integrated a virtualized emulation of *Air Assault* into our terminal emulator shell ([test_wayland_terminal_shell.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_terminal_shell.c)), mapping it alongside our Cimarron spreadsheet interfaces.

---

## 1. Historical Gameplay Mechanics

In the original C64 version, Bob Lloret utilized sprite matrices and character custom fonts to render the descending helicopters:
*   **Player Cannon (`=^=`)**: Placed at the bottom scanline, moving horizontally.
*   **Enemy Aircraft (`v`)**: Slowly sweep left-to-right and step down vertical scanlines.
*   **Missiles (`|`)**: Fired upwards, clearing out enemy lines upon collision detection.
*   **Shield Integrity**: The player has 3 shield units. If an enemy reaches the bottom, shield strength is reduced.

---

## 2. Technical Integration in the Wayland Terminal Shell

We translate Bob Lloret's game loops and sprite collisions into a dedicated console mode (`MODE_AIRASSAULT`):

### 2.1 State variables
*   `g_air_player_x`: Canon offset column (1 to 48).
*   `g_air_missile_x` & `g_air_missile_y`: Missiles active on the coordinate grid.
*   `g_air_invaders_x[5]` & `g_air_invaders_y[5]`: Position states of descending enemy units.

### 2.2 Gameplay Actions
*   **Left / Right Movement**: Keys **`A`** and **`D`**.
*   **Laser Missile Fire**: **`Spacebar`**.
*   **Invader Update Loop**: Updated every 25 game ticks, moving enemy units down. When an enemy hits the baseline, it regenerates at the top and reduces player shields by 1.

---

## 3. Spreadsheet Synchronization (Cimarron Interop)

Reflecting Cimarron Corporation's historic focus on file pipeline interoperability under the Standun Controls umbrella, we have wired the active score metrics directly into the spreadsheet engine. 

*   When playing *Air Assault*, the game score is updated in memory.
*   Upon entering the **Insta-Calc** spreadsheet (command `CALC`), the current score and telemetry values are loaded from registers directly into row 4 cells, allowing developers to analyze performance data or write spreadsheet formulas using active game stats!
