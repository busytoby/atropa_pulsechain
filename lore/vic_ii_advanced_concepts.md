# VIC-II Sprite Graphics: Advanced Hardware Concepts

This guide details the advanced graphics programming techniques and VIC-II register mechanisms introduced in Sally Greenwood Larsen's classic 1983 book *Sprite Graphics for the Commodore 64*.

---

## 1. Multicolor Sprite Architecture ($D01C, $D025, $D026)

In high-resolution mode, a sprite has a resolution of 24x21 pixels and supports only one foreground color (plus transparency). To allow up to three colors, Larsen's book introduces **Multicolor Mode**.

### A. Horizontal Resolution Trade-Off
When multicolor mode is enabled for a sprite (by setting the corresponding bit in `$D01C` / `53276`), adjacent pixels are paired horizontally.
* **Resolution:** Reduced from 24x21 to **12x21** double-width pixels.
* **Color Selection:** Each pixel is represented by **2 bits** of data, allowing four possible states:

| Bit Pair | Color Source | Register / Address | C64 Default Color |
| :--- | :--- | :--- | :--- |
| `00` | **Transparent** | N/A | Current background color |
| `01` | **Sprite Multicolor 1** | `$D025` / `53285` | Orange |
| `10` | **Individual Sprite Color** | `$D027`–`$D02E` | Sprite-specific (e.g. Sprite 0 Color) |
| `11` | **Sprite Multicolor 2** | `$D026` / `53286` | Light Brown |

### B. Shared Colors vs. Sprite Colors
Registers `$D025` and `$D026` are **shared** by all 8 sprites. If you change the color in `$D025`, every multicolor sprite currently displaying bit pair `01` will change color simultaneously. This allows complex multi-sprite scenes to share a unified palette.

---

## 2. Hardware Collision Detection ($D01E, $D01F)

The VIC-II chip contains dedicated hardware logic to detect overlaps, allowing programmers to avoid heavy math calculations in BASIC.

### A. Sprite-to-Sprite Collision ($D01E / 53278)
* **Function:** When the non-transparent pixels of any two enabled sprites overlap on the screen, the VIC-II sets the corresponding bits in `$D01E` to `1`.
* **Bit Assignment:** Bit 0 for Sprite 0, Bit 1 for Sprite 1, etc.
* **Clearing (Strobe):** Reading this register (e.g., `PEEK(53278)`) automatically clears it to `0`. It will not register another collision until the sprites separate and collide again.

### B. Sprite-to-Background Collision ($D01F / 53279)
* **Function:** When any non-transparent pixel of a sprite overlaps with a non-background character pixel (such as a text character or background graphic), the sprite's corresponding bit in `$D01F` is set to `1`.
* **Clearing:** Like `$D01E`, reading `$D01F` resets all bits to `0`.

---

## 3. The 9th-Bit Coordinate Register ($D010 / 53264)

The Commodore 64 screen is 320 pixels wide (plus borders, making the coordinate space span up to 504).
* **The 8-Bit Limitation:** Standard position registers (`$D000`–`$D00F`) are only 8-bit, restricting coordinates to the range `0`–`255`.
* **The Solution:** The VIC-II uses a single register, `$D010` / `53264`, to store the **Most Significant Bit (MSB)** for all 8 sprites:
  * **Bit 0:** 9th coordinate bit for Sprite 0.
  * **Bit 1:** 9th coordinate bit for Sprite 1.
  * ...
  * **Bit 7:** 9th coordinate bit for Sprite 7.
* **Math in BASIC:** To move a sprite past X = 255, BASIC programs must set the MSB:
  ```basic
  POKE 53264, PEEK(53264) OR 1 : REM Set 9th bit for Sprite 0
  POKE 53248, X - 256          : REM Set low-byte coordinate
  ```

---

## 4. Priority Layering ($D01B / 53275)

Register `$D01B` determines the overlapping priority of sprites relative to background character text and bitmap pixels.
* **Bit = 0 (Default):** Sprite is in front of the background.
* **Bit = 1:** Sprite moves behind background characters (except background color `00` pixels). This is useful for making characters walk behind trees, pillars, or walls.

---

## 5. Virtual VIC-II Simulator Implementation

In our web-based interactive tool [sally_larsen_c64.html](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/frontend/sally_larsen_c64.html), these registers are fully emulated:
* **Sprite 0 & Sprite 1 Renderers:** Independent canvases (`sim-canvas-0`, `sim-canvas-1`) with high-fidelity coordinate positioning and size expansions.
* **Pixel-Perfect Overlaps:** Uses offscreen image buffers to calculate physical non-transparent pixel overlap intersections at high speed.
* **Visual Status Indicators:** Emulates registers `$D01E` and `$D01F` in real-time, displaying hexadecimal register values and triggering red status LED lights for active sprite collision channels.
* **Latch Clear (Strobe):** Emulates the strobe latch behavior by clearing registers back to `$00` only when the user invokes the **PEEK & Clear** instruction.
* **9th-Bit MSB coordinate register ($D010 / 53264):** Displays the binary state of the horizontal coordinate overflow in real-time with green LED indicators that trigger whenever Sprite 0 or Sprite 1 coordinates cross the 255-pixel threshold.
