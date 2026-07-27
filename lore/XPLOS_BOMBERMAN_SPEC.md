# Auncient Bomberman on XplOS Specification

This document defines the architecture and state loop for **Auncient Bomberman**, an interactive game designed to execute as cooperative tasks on the XplOS microkernel.

---

## 1. Game State Structure

The game state is managed in a compact structure fitting withinNamespaced storage allocations:

```xpl
/* XplOS Bomberman Grid and Agent Layout */
DECLARE GRID_WIDTH LITERAL '16', GRID_HEIGHT LITERAL '16';
DECLARE TILE_EMPTY LITERAL '0', TILE_WALL LITERAL '1', TILE_BOMB LITERAL '2', TILE_BLAST LITERAL '3';

DECLARE BOMBERMAN_GAME STRUCTURE(
    PLAYER_X FIXED,
    PLAYER_Y FIXED,
    BOMB_X FIXED,
    BOMB_Y FIXED,
    BOMB_TIMER FIXED,
    ACTIVE_BOMBS FIXED,
    GRID(256) BYTE
);
```

---

## 2. Interactive Loop & SCSI Control Mapping

In accordance with system low-level driver routing constraints, player inputs interface via the loopback keyboard dispatcher:

* **Keycode `30` (A/a):** Move Player Left ($X = X - 1$).
* **Keycode `32` (D/d):** Move Player Right ($X = X + 1$).
* **Keycode `17` (W/w):** Move Player Up ($Y = Y - 1$).
* **Keycode `31` (S/s):** Move Player Down ($Y = Y + 1$).
* **Keycode `57` (Space):** Place Bomb at current tile coordinates.

---

## 3. Co-routine Task Scheduling

The game executes inside the scheduler queue as two separate cooperative tasks:

1. **`tsfi_bomberman_update_task` (Game Loop):**
   * Processes bomb timers, handles explosion propagation (blast tiles) lasting 3 ticks, checks collision flags, and yields (`tsfi_xplos_yield`).
2. **`tsfi_bomberman_render_task` (Draw Loop):**
   * Rasterizes the 16x16 grid to the 256x256 VRAM framebuffer by scaling each tile to a 16x16 pixel block. It maps:
     * Walls to Grey.
     * Bombs to Red flashing dots.
     * Blasts to bright Yellow cells.
     * Player to a Blue coordinate crosshair.
   * Swaps double-buffered active frames upon completing raster sweeps.
