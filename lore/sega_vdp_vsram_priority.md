# Sega Genesis VDP VSRAM & Priority Layering

The **Sega Genesis VDP** provides advanced layout and depth configuration features. This document details the inner workings of **VSRAM (Vertical Scroll RAM)** column scrolling and **Tile Priority** composition.

---

## 1. VSRAM Column Scrolling (2-Cell Mode)

The VDP contains 80 bytes of dedicated internal memory called **VSRAM**.
* **Column Granularity**: Unlike horizontal scrolling (which can scroll individual scanlines), vertical scrolling operates on **2-cell wide columns** (16 pixels).
* **Grid Layout**: For a screen width of 320 pixels (40 tiles), there are exactly **20 vertical scroll columns**.
* **Use Cases**: By setting different vertical scroll values for each column, games can simulate curved surfaces, water rippling vertically, or pseudo-3D scaling roads.

---

## 2. Tile Priority Layering

Each tile entry in the Plane A and Plane B scroll maps contains a **Priority Bit** (Bit 15 of the 16-bit Tile Attribute word).

When the VDP compositor outputs a pixel, it decides which color to display based on the layer hierarchy and priority flags:

```
┌────────────────────────────────────────────────────────┐
│ 1. Plane A (High Priority)                             │
├────────────────────────────────────────────────────────┤
│ 2. Plane B (High Priority)                             │
├────────────────────────────────────────────────────────┤
│ 3. Sprites (High Priority)                             │
├────────────────────────────────────────────────────────┤
│ 4. Plane A (Low Priority)                              │
├────────────────────────────────────────────────────────┤
│ 5. Plane B (Low Priority)                              │
├────────────────────────────────────────────────────────┤
│ 6. Sprites (Low Priority)                              │
└────────────────────────────────────────────────────────┘
```

If a background tile is marked High Priority, it renders *in front of* Low Priority sprites. This enables complex graphics layering, such as making a character walk behind a foreground pillar or foliage.

---

## 3. Simulation Implementation

To demonstrate this system, we have updated the simulator script: [test_sega_vdp.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_sega_vdp.js).

This script:
1. Simulates VSRAM containing 20 column offsets.
2. Implements a column-by-column compositor that resolves pixel sources based on horizontal line scrolls, vertical column scrolls, and tile priorities.
