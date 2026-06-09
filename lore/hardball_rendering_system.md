# C64 Hardball Wayland-Vulkan Presentation System

This document details the architectural layout of the C64 *Hardball!* (1985) rendering simulator. The system uses a native **Wayland + Vulkan** presentation shell to display a C64-style retro baseball screen featuring the iconic pitcher's perspective.

---

## 1. Screen Layout & Geometry

```
  ┌────────────────────────────────────────────────────────┐
  │ SCOREBOARD:   VIS 0  HOME 2   INNING 5   S:0 B:1 O:2   │
  ├────────────────────────────────────────────────────────┤
  │                                                        │
  │                   / \  Outfield Wall                   │
  │                  /   \                                 │
  │                 /     \  Infield Foul Lines            │
  │                /   O   \  Pitcher                      │
  │               /    o    \  Moving Ball                 │
  │              /    / \    \                             │
  │             /    O   O    \  Catcher & Batter          │
  │            /  [Home Plate] \                           │
  └────────────────────────────────────────────────────────┘
```

The rendering grid mimics the 4:3 display ratio of standard CRT monitors:
1. **Scoreboard Header**: Static text and numbers tracking status.
2. **Perspective Foul Lines**: Screen coordinates projected from a focal vanishing point at the top center of the field down to the bottom corners.
3. **Player Sprites**:
   - **Pitcher**: Centered in the middle of the field (mound).
   - **Batter**: Standing to the left of home plate.
   - **Catcher**: Squatting directly behind home plate.
4. **Interactive Ball Path**: A particle vector shifting from the pitcher's hand towards the strike zone, scaling in size (z-depth simulation) per frame.

---

## 2. Compilation Target

The simulator is located at [test_wayland_hardball.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_wayland_hardball.c).

To compile and run it:
```bash
make tests/test_wayland_hardball
./tests/test_wayland_hardball
```
*Note: This utilizes our local `VulkanSystem` shell to hook into Wayland protocols and output frames directly into a presented presentation viewport.*
