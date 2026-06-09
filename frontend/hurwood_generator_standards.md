# Hurwood Program Generator Standards & Schemas

This document defines the standard specification and memory layout schemas for the **Hurwood Program Generator** within the terminal-shell console interface. These schemas standardize the generation of C64 BASIC statements to automate sound design, sprite configuration, and procedural graphic mazes.

---

## 1. Schema Specifications

### 1.1 `MAZE` (Procedural Character Graphics)
Standardizes high-speed random character graphic display sequences to output random slopes, drawing vector-like labyrinth layouts.

*   **Standard Code Block**:
    ```basic
    10 PRINT "CLR": REM CLEAR SCREEN (CHR$ 147)
    20 POKE 53280, 0: POKE 53281, 0: REM BLACK BORDERS/BACKGROUND
    30 FOR I = 1 TO 1000
    40 R = INT(RND(1)*2)
    50 IF R = 0 THEN PRINT "/";: GOTO 70
    60 PRINT "\";
    70 NEXT I
    ```

---

## 2. Sound Register Map (`SOUND` Schema)

The sound generator targets the Commodore 64 SID chip register block starting at `$D400` (54272):

| Memory Address (decimal) | SID Function | Value Range | Standard Assignment |
| :--- | :--- | :--- | :--- |
| `54272` | Voice 1 Low Frequency | `0–255` | `37` (Freq C-4 low byte) |
| `54273` | Voice 1 High Frequency | `0–255` | `17` (Freq C-4 high byte) |
| `54276` | Voice 1 Control Register | `0–255` | `17` (Start Triangle) / `16` (Stop) |
| `54277` | Voice 1 Attack/Decay | `0–255` | `15` (Fast Attack, Medium Decay) |
| `54278` | Voice 1 Sustain/Release | `0–255` | `240` (High Sustain) |
| `54290` | SID Volume / Filter | `0–15` | `15` (Maximum volume output) |

---

## 3. Sprite Configuration Map (`SPRITE` Schema)

The sprite generator configures the VIC-II registers starting at `$D000` (53248) and sprite pointer areas:

| Memory Address (decimal) | VIC-II Function | Value Range | Standard Assignment |
| :--- | :--- | :--- | :--- |
| `2040` | Sprite 0 Pointer | `0–255` | `13` (Points to block `$0340` in RAM) |
| `832`–`895` | Sprite Data Area | `0–255` | `255` (Creates 24x21 solid visual mask) |
| `53248` | Sprite 0 X Coordinate | `0–255` | `100` |
| `53249` | Sprite 0 Y Coordinate | `0–255` | `100` |
| `53269` | Sprite Enable Register | `0–255` | `1` (Enables Sprite 0) |
| `53287` | Sprite 0 Color Register | `0–15` | `1` (Standard White highlight) |

---

## 4. Custom Character Redefinition Map (`CHARSET` Schema)

The custom character generator copies character glyph matrices from ROM (at `$D000`) to RAM (at `$3000`), switching interrupt and bank selections:

| Memory Address (decimal) | Hardware Function | Value Range | Standard Assignment |
| :--- | :--- | :--- | :--- |
| `1` | 6510 Processor Port (RAM/ROM Banking) | `0–255` | Clear bit 2 to bank in Char ROM, Set bit 2 to restore RAM |
| `52` / `56` | BASIC RAM Limit Pointer | `0–255` | Set to `48` to restrict BASIC memory below `$3000` (12288) |
| `12288`–`14335` | Custom Character RAM | `0–255` | Destination area for 256 custom character glyphs |
| `53248`–`55295` | Character Generator ROM | `0–255` | Source area for ROM glyph patterns |
| `53272` | VIC-II Memory Control Register | `0–255` | Or-in `12` to point VIC-II custom set to `$3000` |
| `56334` | CIA 1 Control Register A (Interrupt Control) | `0–255` | Disable timer interrupts during copy, re-enable after |

---

## 5. Raster Split-Screen Interrupt Map (`RASTER` Schema)

The raster interrupt generator configures split-screen interrupts at a specific scanline:

| Memory Address (decimal) | Hardware Function | Value Range | Standard Assignment |
| :--- | :--- | :--- | :--- |
| `788`–`789` | CPU Interrupt Vector | `16-bit address` | Redirects to `$0D00` (3328) for the custom interrupt handler |
| `53265` | VIC-II Control Register 1 | `0–255` | Clear bit 7 to set raster interrupt target scanline < 256 |
| `53266` | VIC-II Raster Compare Register | `0–255` | Set to `120` (target scanline for background/border change) |
| `53274` | VIC-II Interrupt Enable Register | `0–255` | Set bit 0 (1) to enable raster interrupts |
| `53273` | VIC-II Interrupt Status Register | `0–255` | Write bit 0 (1) inside ISR to acknowledge and clear interrupt |
| `56333` | CIA 1 Interrupt Control Register | `0–255` | Set to `127` ($7F) to disable timer interrupts and prioritize raster |

---

## 6. Joystick Port Scanning Map (`JOYSTICK` Schema)

The joystick scanner reads direct pin states from the CIA 1 complex:

| Memory Address (decimal) | Hardware Function | Value Range | Standard Assignment |
| :--- | :--- | :--- | :--- |
| `56320` | CIA 1 Data Port A (Joystick 2) | `0–255` | Bit 0 (Up), Bit 1 (Down), Bit 2 (Left), Bit 3 (Right), Bit 4 (Fire) [Active Low: 0 = Pressed] |
| `56321` | CIA 1 Data Port B (Joystick 1) | `0–255` | Direct line states for Joystick 1 |

---

## 7. Code Compaction Rules (`COMPACT` Mode)

To maximize virtual memory efficiency and emulation execution speed, the `COMPACT` modifier automatically strips non-essential characters from BASIC listings:

1. **Whitespace Elimination**: Removes all space characters (`$20`) except:
   - When located inside quoted string literals (e.g. `PRINT "CLR"`).
   - When located inside `REM` (comments) to preserve readability of annotations.
2. **Keyword packing**: Consolidates statements so syntax parsers interpret commands without boundary padding (e.g., `FOR I = 1 TO 1000` is packed into `FORI=1TO1000`).
3. **Execution**: Append `COMPACT` as the second argument to any command to activate compaction (e.g., `HURWOOD SOUND COMPACT`).

---

## 8. Kwitowski-Harris Sound Concept Schema (`CONCEPT` Schema)

The Kwitowski-Harris Sound Concept utilizes a structured **three-byte memory block** to define independent sound sequences mapped to standard VIC-20 sound registers (as documented by Neal Harris):

### 8.1 3-Byte Sound Data Structure
Each sound unit is declared in a sequential row of 3 bytes:

| Byte Offset | Parameter | Value Range | Description |
| :--- | :--- | :--- | :--- |
| `Byte 0` | Voice Index | `10–13` | Voice 1 (10), Voice 2 (11), Voice 3 (12), Voice 4/Noise (13) |
| `Byte 1` | Pitch / Frequency | `128–255` | Frequency value loaded into target voice register |
| `Byte 2` | Duration / Gate | `0–255` | Playback duration tick count |

### 8.2 VIC-20 Sound Hardware Mappings
The playback driver redirects sound offsets to the VIC-20 audio registry block:

| Memory Address (decimal) | Register Function | Register Offset Range |
| :--- | :--- | :--- |
| `36874` | Voice 1 (Bass/Alto) | Offset 0 (Voice Index 10) |
| `36875` | Voice 2 (Tenor) | Offset 1 (Voice Index 11) |
| `36876` | Voice 3 (Soprano) | Offset 2 (Voice Index 12) |
| `36877` | Voice 4 (Noise) | Offset 3 (Voice Index 13) |
| `36878` | Master Volume Control | Low 4 bits (`0–15`) control global volume |

### 8.3 Sounder Machine Code Playback
When running the `STAGE` operation for the `CONCEPT` schema, a compiled 6502 assembly playback engine is staged in memory:

*   **Playback Code Address**: `$0D00` (3328 decimal).
*   **Target Sound Queue Address**: `$1000` (4096 decimal).
*   **Assembly Hook**: The staged BASIC program calls `SYS 3328`, invoking the Sounder binary. It reads the 3-byte cells from `$1000`, configures the voice frequency mappings, decrements loop counters using an internal delay routine, and terminates when a duration value of `0` is reached.

