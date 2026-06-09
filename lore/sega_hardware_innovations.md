# Sega Consumer Products — Hardware & Video Architecture

Sega's hardware history is defined by highly modular, multi-coprocessor designs. From the 8-bit Sega Master System up to the 128-bit Dreamcast, Sega's engineering focused on massive sprite rendering and complex background scrolling plane layers.

This document details the video and audio mechanics of Sega's flagship systems: the **Sega Genesis (Mega Drive)** and the **Sega Saturn**.

---

## 1. Sega Genesis / Mega Drive VDP Architecture

The Sega Genesis Video Display Processor (VDP) managed graphics in parallel with the Motorola 68000 CPU.

### 1.1 Dual Scroll Planes (Plane A & Plane B)
Unlike simple tile layouts, the Genesis VDP supports two completely independent scrolling background layers: **Scroll Plane A** and **Scroll Plane B**. 
* **Window Plane**: A subset of Plane A can be locked as a non-scrolling Window (useful for HUDs/status bars) with zero CPU drawing overhead.
* **Priority Mapping**: Each tile in Plane A or Plane B has a priority bit. When high-priority tiles are drawn, they cover sprites, allowing sprites to pass behind background elements (e.g. walking behind trees).

### 1.2 Line Scrolling (Raster Effects)
The VDP allows changing the horizontal scroll offset of the background plane *on a per-scanline basis*:
* **Mode Selection**: Scroll values can be set for the entire screen, per tile row (8 pixels), or per individual scanline (1 pixel).
* **Parallax Scrolling**: By offsetting each scanline slightly, games (such as *Sonic the Hedgehog*) simulate complex 3D depth and parallax curves for horizons and water surfaces.

---

## 2. Sega Saturn: The VDP1 & VDP2 Split

The Sega Saturn's architecture is famous for its complexity, relying on two distinct custom GPUs:

* **VDP1 (Sprite & Polygon Processor)**: Renders sprites, textured quadrilaterals (Saturn does not use triangles), and wireframes into a dual frame buffer.
* **VDP2 (Background Processor)**: An independent processor that composites the VDP1 sprite frame buffer with up to 5 background planes. VDP2 handles rotation, scaling, and perspective texture mapping on these planes (similar to Nintendo's Mode 7 but in multiple layers).

---

## 3. Sega Audio: Yamaha YM2612 FM Synthesis

The Genesis features the **Yamaha YM2612** Frequency Modulation (FM) sound synthesizer chip.
* **Operators & Channels**: 6 FM voices, each driven by 4 operators (sine wave generators that modulate each other's frequency).
* **Feedback & Algorithms**: Different configurations (algorithms) of these operators yield brassy, metallic, or synthesized waveforms.
* **DAC Mode**: Channel 6 can act as an 8-bit DAC to play digital PCM samples (e.g. digitized speech like the classic "SEGA!" sound).

---

## 4. Emulation Verification

Our automated testing framework at [tests/test_terminal_exploratory.py](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_terminal_exploratory.py) verifies these architectural models:
- **Genesis VDP (SEGAVDP)**: Validates scrolling plane layout priority logic and scanline raster effects.
- **Saturn VDP1 (SATURNVDP)**: Validates distorted quad rendering calculations and bilinear UV coordinates.
These tests run successfully under mock CPU execution inside the terminal dashboard.
