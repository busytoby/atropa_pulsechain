# Full-Stack Vulkan & Audio Datamost Integration Plan

This document details the architecture to integrate our Yul-based EVM emulator contracts (**audio** and **graphics**) into the native **Vulkan/Wayland display subsystem** (`tsfi_vulkan.c`) for a live retro-computing visualizer.

---

## 1. Full-Stack Data Pipeline Architecture

The system operates as a hybrid pipeline where the **decentralized EVM** acts as the deterministic hard-wired motherboard, and the **C/Vulkan application** acts as the hardware visual/audio output driver:

```mermaid
graph TD
    subgraph EVM Motherboard (Anvil JSON-RPC)
        Kernel[Microkernel Router]
        SID[musicMaker: Voice Frequencies]
        VIC[graphicsSystem: Sprite Coords & Collisions]
    end

    subgraph Native Host Drivers (C / C++)
        RPC[EVM Poller / Event Listener]
        AudioEngine[OpenAL / SDL Audio Oscillator]
        VulkanEngine[Vulkan Render Pipeline: vulkan_render.c]
    end

    subgraph Output Monitors
        Speaker((Audio Output: Wave Synthesizer))
        Display((Vulkan/Wayland Display Monitor))
    end

    RPC -->|Query Registers| SID
    RPC -->|Query Registers| VIC
    RPC -->|Synthesize Waves| AudioEngine
    RPC -->|Draw Sprites| VulkanEngine
    AudioEngine --> Speaker
    VulkanEngine --> Display
```

---

## 2. Audio Integration: Waveform Resynthesis

In `musicMaker.yul`, frequency registers for the virtual SID chip are mapped to memory slots (e.g. `54272` and `54273`).

### Integration Steps:
1. **Poll Audio State:** The C host query loop fetches current values from the `musicMaker` registers via `getVoice1Frequency()`.
2. **Dynamic Oscillators:** In the audio thread, a standard waveform oscillator (supporting Sine, Triangle, Sawtooth, and Noise waveforms) uses this frequency to synthesize PCM samples in real time:
   $$y(t) = A \cdot \sin(2\pi \cdot f_{\text{EVM}} \cdot t)$$
3. **Playback Buffer:** Samples are pushed directly to a standard audio queue (using `SDL_QueueAudio` or `ALSA`) to generate live sound outputs matching the synthesized MIDI sequence.

---

## 3. Visual Integration: Vulkan Sprite Rendering

In `graphicsSystem.yul`, sprite pointers and coordinates are mapped to slots simulating VIC-II registers (e.g. `53248` to `53263`).

### Integration Steps:
1. **Poll Sprite Layout:** The host fetches active sprites and their $(X, Y)$ coordinates from `graphicsSystem` via `checkCollisions()`.
2. **Vulkan Pipeline Bindings:** In `vulkan_render.c`, sprite geometries are represented as 2D textured quads. The vertex shader maps normalized device coordinates (NDC) dynamically using the polled pixel coordinates:
   $$\text{NDC}_x = \frac{X_{\text{Sprite}} - 160}{160}, \quad \text{NDC}_y = \frac{100 - Y_{\text{Sprite}}}{100}$$
3. **Texture Atlas Mapping:** The fragment shader samples retro C64 font/sprite sheets mapped from the RDNA4 staging buffers to display the sprite patterns.
4. **Collision Indicator Overlay:** If a collision bit is set in the polled mask, the Vulkan render pass dynamically overlays a glowing bounding box around the colliding sprites.

---

## 4. Oscilloscope & Status Hud Layout

The Vulkan interface displays a three-pane retro console display:

| Screen Pane | Contents | Rendering Method |
| :--- | :--- | :--- |
| **Active Playfield** | The 8 VIC-II Sprites moving based on EVM coordinates. | Vulkan Texture Pipelines & Bounding Box Overlays. |
| **SID Oscilloscope** | Live wave representation of the voice frequencies. | Dynamic vertex buffer line-strips (`VK_PRIMITIVE_TOPOLOGY_LINE_STRIP`). |
| **CPU State Monitor** | Real-time dump of `cpu6502` registers (`A`, `X`, `Y`, `SR`). | Text-render glyphs mapping status codes from `getCPUState()`. |
