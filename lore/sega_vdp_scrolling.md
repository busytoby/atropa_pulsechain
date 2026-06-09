# Sega VDP Scroll Planes & Parallax Line Scrolling

The **Sega Genesis VDP (Video Display Processor)** is capable of advanced layer rendering and multi-plane layout composition. This document details the architectural layout of background planes and the mechanics of raster line scrolling.

---

## 1. Plane Configurations

The VDP composites graphics from three primary background layers:

```
                  [ Composite Video Output ]
                              ▲
                              │
                    ┌─────────┴─────────┐
                    │  Window Plane     │ (Static Overlay HUD)
                    └─────────┬─────────┘
                              │
                    ┌─────────┴─────────┐
                    │  Scroll Plane A   │ (Foreground Playfield)
                    └─────────┬─────────┘
                              │
                    ┌─────────┴─────────┐
                    │  Scroll Plane B   │ (Background/Far scenery)
                    └───────────────────┘
```

* **Plane A**: Used for the primary interactive environment.
* **Plane B**: Used for distant backgrounds, clouds, mountains.
* **Window**: Replaces a designated rectangular section of Plane A, ignoring scroll registers (useful for static status bars and menus).

---

## 2. Parallax Line Scrolling Mechanics

To simulate pseudo-3D depth, the VDP can operate in **Line Scroll Mode**.
* **HScroll Table**: The VDP reads horizontal scroll offsets from a dedicated table in VRAM.
* **Scanline Resolution**: In line scroll mode, the HScroll table contains 224 distinct 16-bit offset values—one for each horizontal scanline.
* **Parallax Formula**: The scroll offset for scanline $y$ is computed as:
  $$Offset(y) = BaseOffset \times SpeedFactor(y)$$
  By scaling the $SpeedFactor$ according to the scanline depth (e.g. background hills scroll slower, foreground grass scrolls faster), the hardware displays realistic parallax movement.

---

## 3. Simulation Implementation

To demonstrate this system, we have created a simulator script: [test_sega_vdp.js](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/scripts/test_sega_vdp.js).

This script:
1. Simulates the VRAM scroll table setup.
2. Calculates scanline-by-scanline offsets using a parallax depth equation.
3. Simulates VDP scanline rendering output, including the static Window Plane override.
