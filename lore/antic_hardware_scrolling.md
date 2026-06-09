# Atari ANTIC Hardware Scrolling

The **Atari ANTIC** graphics coprocessor features hardware-level horizontal and vertical scrolling. By using fine scroll registers combined with dynamic display list adjustments, games can achieve fluid, sub-pixel camera movements with very little CPU usage.

---

## 1. Scrolling Register Maps

ANTIC exposes two 8-bit write-only registers for fine scroll adjustments:

### 1.1 Horizontal Scroll Register (`HSCROL`)
* **Address**: `$D404`
* **Resolution**: 4-bit (values `0` to `15` clock cycles/pixels)
* **Function**: Delays the start of the scanline rendering clock to shift pixels horizontally. Shifting higher values moves the playfield to the right.

### 1.2 Vertical Scroll Register (`VSCROL`)
* **Address**: `$D405`
* **Resolution**: 3-bit (values `0` to `7` scanlines)
* **Function**: Shifts the display screen downward by up to 7 scanlines.

---

## 2. Coarse vs. Fine Scrolling Mechanics

Because `HSCROL` and `VSCROL` can only shift the viewport by a fraction of a single tile/character cell, continuous scrolling requires coordinating **fine scrolling** and **coarse scrolling**:

```
[ Camera Position (X, Y) ]
           │
           ├─► Fine Scroll = Position % CellSize  ──► Write to HSCROL / VSCROL
           │
           └─► Coarse Offset = Position / CellSize ─► Update Display List LMS Address
```

### Display List LMS Instruction
When horizontal or vertical scrolling is enabled, ANTIC reads 2 extra bytes in the Display List instruction (LMS - Load Memory Scan) to locate the display data. As the camera moves, the CPU updates these LMS addresses to step through the larger game map stored in RAM.

---

## 3. Simulation Implementation

To demonstrate this system, we have created a simulator script: [test_antic_scrolling.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_antic_scrolling.js).

This script:
1. Defines a tilemap world grid.
2. Calculates the required `HSCROL` / `VSCROL` fine offset values based on a simulated continuous camera path.
3. Calculates the coarse memory offset and updates the virtual Display List LMS address pointers.
4. Outputs the final viewport viewport boundaries showing seamless panning.
