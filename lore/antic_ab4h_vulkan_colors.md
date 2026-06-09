# Supporting AB4H Color in Vulkan Displays via ANTIC Display Lists

The **AB4H format (`VK_FORMAT_R16G16B16A16_SFLOAT`)** is a high-dynamic-range (HDR) color format utilizing 16-bit floating-point values for each channel (Red, Green, Blue, Alpha), requiring **64 bits (8 bytes) per pixel**.

While AB4H provides extreme color fidelity, transferring raw 64-bit framebuffers between CPU memory and GPU VRAM at high resolutions (e.g. 60fps) introduces massive memory bandwidth bottlenecks.

By adapting the **Atari ANTIC Display List** and **Color Look-Up Table (CLUT)** architecture, we can support high-fidelity AB4H colors in our Vulkan display pipeline while keeping processing and bandwidth costs extremely low.

---

## 1. The Indexed Bandwidth Bottleneck Solution

Instead of the CPU writing 64-bit float values for every pixel, the Vulkan display pipeline is redesigned using an indexed color lookup:

```
  ┌────────────────────────────────────────────────────────┐
  │                   Host CPU (ZMM VM)                    │
  │   Writes 8-bit pixel indices (R8_UINT) & Display List   │
  └──────────────────────────┬─────────────────────────────┘
                             │ (1 Byte/Pixel = 8x Bandwidth Saving!)
  ┌──────────────────────────▼─────────────────────────────┐
  │                    Vulkan GPU VRAM                     │
  │   [8-bit Texture Buffer]   [AB4H Palette Uniform (UBO)]│
  └────────────────────┬───────────────┬───────────────────┘
                       │               │
  ┌────────────────────▼───────────────▼───────────────────┐
  │                 Vulkan Fragment Shader                 │
  │   Indices read from texture index into uniform palette  │
  └──────────────────────────┬─────────────────────────────┘
                             │
  ┌──────────────────────────▼─────────────────────────────┐
  │                 AB4H Vulkan Framebuffer                │
  │   Outputs final 64-bit R16G16B16A16_SFLOAT pixels      │
  └────────────────────────────────────────────────────────┘
```

### 1.1 Indexed Texture Streaming (8x Bandwidth Reduction)
* **Standard AB4H Transfer:** A $1024 \times 768$ framebuffer at 64-bit color requires **6.29 MB** per frame. At 60 FPS, this demands **377.4 MB/s** of transfer bandwidth.
* **ANTIC Indexed Transfer:** The screen RAM is stored and streamed as `VK_FORMAT_R8_UINT` (8-bit index). The same $1024 \times 768$ frame requires only **786 KB**. At 60 FPS, this drops the bandwidth to **47.1 MB/s** (an **8x reduction**).
* **Shader Expansion:** The Vulkan fragment shader samples the 8-bit texture index and resolves the final 64-bit AB4H color on-the-fly by querying a Uniform Buffer Object (UBO) holding the color palette array.

---

## 2. Scanline-Specific Palette Swapping (DLI Emulation)

In a standard index-color system, you are limited to a global palette (e.g., 256 colors). To break this barrier without adding overhead, we adapt **Display List Interrupts (DLI)** from ANTIC:

1. **Instruction-Level Palette Updates:** The Display List program parsed by the Yul engine contains palette-load instructions:
   * `LOAD_PALETTE <StartScanline>, <PaletteIndex>, <AB4H_Color>`
2. **GPU Uniform Array Swapping:** As the Vulkan renderer draws the screen, the vertex shader selects different ranges of the palette uniform array based on the current scanline coordinate (`gl_InstanceIndex` or push constants).
3. **Result:** We can display thousands of distinct AB4H colors on screen simultaneously (e.g., having a completely different set of 256 colors for the header, terminal output, and footer) while maintaining a strict 8-bit index framebuffer transfer budget.

---

## 3. Vulkan Fragment Shader Implementation (GLSL)

The GLSL shader below demonstrates how the index texture is mapped to the AB4H palette uniform buffer:

```glsl
#version 450

layout(binding = 0) uniform sampler2D screenIndexTexture; // 8-bit texture indices
layout(binding = 1) uniform PaletteBuffer {
    vec4 ab4hColors[256]; // 256-entry AB4H color palette
} palette;

layout(location = 0) in vec2 fragTexCoord;
layout(location = 0) out vec4 outColor; // Output as R16G16B16A16_SFLOAT

void main() {
    // 1. Fetch 8-bit index value (0.0 to 1.0 range mapped by sampler)
    float indexRaw = texture(screenIndexTexture, fragTexCoord).r;
    int colorIndex = int(indexRaw * 255.0);
    
    // 2. Perform low-cost O(1) array lookup
    outColor = palette.ab4hColors[colorIndex];
}
```
