# Playing Epyx Titles on the Native Vulkan Pipeline: Technical Exploration

This document outlines the design architecture to execute and display Epyx retro titles (e.g., *Crush, Crumble and Chomp!*, *The Temple of Apshai*) using our Yul-based EVM emulator and the native Vulkan rendering plugin (`tsfi_vulkan.c`).

---

## 1. Graphics Pipeline: Assembling the Monsters

Unlike simpler games like *Pong* which use single hardware sprites, Epyx games like *Crush, Crumble and Chomp!* represent large entities (such as giant monsters: Glogg, Arachnis, or the Kraken) using multiple coordinated structures:

1. **Sprite Multiplexing**:
   * The monster is assembled from multiple overlapping C64 sprites to achieve rich multi-color details.
   * **Vulkan Resolution**: The host polling engine groups sprites `0`–`3` by tracking their relative coordinate offsets. Instead of rendering four independent textured quads, Vulkan combines them into a single instanced draw call using a unified model matrix:
     $$\mathbf{M}_{\text{Monster}} = \mathbf{T}(x_{\text{base}}, y_{\text{base}}) \cdot \mathbf{S}(w, h)$$
2. **Custom Character-Cell Graphics**:
   * Map tiles (San Francisco, Tokyo) are composed of character cells in Screen RAM (`1024`–`2047`).
   * **Vulkan Resolution**: The Vulkan fragment shader samples a custom retro C64 font atlas texture. Character indices read from the EVM's `getScreenRAM()` function map directly to texture coordinates, rendering grid environments with a retro phosphor curvature filter.

---

## 2. On-Chain Collision & Destruction Telemetry

In *Crush, Crumble and Chomp!*, the player stomps on buildings, causing destruction:

* **Sprite-to-Background Collisions**:
  * Regulated via VIC-II register `$D01F` (`53279`), emulated inside `cpu6502.yul`.
* **Vulkan FX Overlay**:
  * When a collision between the monster sprite and a building character cell is flagged on-chain, the Vulkan render pass dynamically registers the event and overlays a particle explosion effect on the corresponding map coordinates.
  * Damage status registers are polled and displayed via a retro HUD panel rendered at the bottom of the viewport using text glyphs.

---

## 3. Real-Time Audio Synchronization (SID to OpenAL)

Sound effects for stomping, military sirens, and crushing structures are modulated via the virtual SID registers in the EVM contract:

- **Stomping/Sirens**: Synthesized via the low-frequency oscillators of Voice 1 and Voice 2 in [musicMaker.yul](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/solidity/bin/musicMaker.yul).
- **Vulkan Synchronization**: The Vulkan rendering loop is locked to the audio callback rate (e.g., 60 FPS synced to a 48kHz audio buffer queue) to ensure visual explosions align perfectly with synthesized audio effects.

---

## 4. Run Command for Native Vulkan Visuals

Developers can run the native CLI visualizer with the Vulkan plugin loaded using the following test pipeline command:

```bash
(echo "LOAD plugins/tsfi_vulkan"; sleep 1; echo "1.0 OPEN_WINDOW"; sleep 10; echo "0.0 EXIT") | ./tsfi2 --cli
```
