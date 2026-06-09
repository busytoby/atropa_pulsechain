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
