# Sally Greenwood Larsen: C64 Sprite Graphics & VIC-II Simulator Reference

This reference documents the hardware registers, pointers, and memory layout from the 1983 classic book *Sprite Graphics for the Commodore 64* by Sally Greenwood Larsen, mapped to the implementations in our game and frontend simulation.

## 1. Commodore 64 VIC-II Sprite Hardware Mapping

| VIC-II Address | Register Type | C64 Purpose | Virtual Emulation / Simulator Equivalent |
| :--- | :--- | :--- | :--- |
| `$D000`–`$D00F` | **X/Y Coordinates** | Position coordinates for Sprites 0–7. | `vic_sprite_x[i]` and `vic_sprite_y[i]` in [test_wayland_smurfs.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_smurfs.c#L282-L334) |
| `$D010` | **MSB (9th bit)** | Coordinates X > 255. | `vic_sprite_msb` |
| `$D015` | **Sprite Enable** | Active sprites indicator. | `vic_sprite_enable` |
| `$D017` | **Y-Expand** | Double height scale. | `vic_sprite_y_expand` (virtual scale mapping) |
| `$D01B` | **Priority** | Render ordering (in front/behind background). | `vic_sprite_priority` (background priority mapped) |
| `$D01C` | **Multicolor** | Toggle 2-bit color pairing. | `vic_sprite_multicolor` |
| `$D01D` | **X-Expand** | Double width scale. | `vic_sprite_x_expand` |
| `$D01E` | **Collision** | Sprite-to-Sprite Collision Strobe. | `vic_sprite_collision` (mirrors Yul hit detection slot 55046) |
| `$07F8`–`$07FF` | **Sprite Pointers** | Sprite data block indexes. | `vic_sprite_pointer[i]` (mirrors active animation page blocks) |

---

## 2. Integrated Technologies

### A. Web Application Simulator
* **Location**: [sally_larsen_c64.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/sally_larsen_c64.html)
* **Features**:
  * Real-time 24x21 drawing matrix.
  * Hi-res (1-bit) and Multicolor (2-bit paired columns) modes.
  * Translation tools: Shift, Flip, Invert, Clear.
  * Interactive VIC-II Register dashboard syncing Position, Enable, Expansion, and Multicolor switches.
  * Simulated C64 CRT screen showing live render results.
  * Real-time C64 BASIC launcher and 63-byte `DATA` block compiler.

### B. EVM Memory Registers Sync
* **Location**: [test_wayland_smurfs.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_smurfs.c#L282-L334)
* **Function**: `sync_c64_vic_registers` translates real-time Yul/EVM variables into virtual 6502 RAM registers during every frame loop, verifying playability, expansion scaling, and collision offsets.
