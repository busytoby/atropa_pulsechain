# Multi-Color Sprite Plotting in the Yul Emulator and Vulkan UI

This document details the architectural design and bit-decoding mechanics for emulating the Commodore 64's **Multi-Color Sprite mode** inside our Yul EVM graphics system and native Vulkan renderer (`vulkan_logic.c`).

---

## 1. Multi-Color Sprite Control Registers

By default, VIC-II sprites are single-color: a $24 \times 21$ bitmap grid where each bit represents either transparency ($0$) or the sprite's primary color ($1$).

In **Multi-Color Sprite mode**, the horizontal resolution is halved to $12 \times 21$ pixels, where each pixel is represented by a **2-bit pair** to select from four color sources:

| Register Address (Hex) | Register Address (Dec) | Function | Description |
| :--- | :--- | :--- | :--- |
| `$D01C` | `53276` | Sprite Multi-Color Register | Setting bit $i$ (0-7) to `1` enables multi-color mode for Sprite $i$. |
| `$D025` | `53285` | Sprite Extra Color 1 | Defines the RGB color mapped to bit pair `01`. |
| `$D026` | `53286` | Sprite Extra Color 2 | Defines the RGB color mapped to bit pair `11`. |
| `$D027 + i` | `53287 + i` | Sprite $i$ Main Color | Defines the RGB color mapped to bit pair `10`. |

---

## 2. Bit-Pair Decoding Logic

In multi-color mode, the $24$-bit row pattern data is split into $12$ distinct pixel pairs ($C_0 C_1$) processed sequentially from left to right:

$$\text{Pixel Color} = \text{Decode}(C_0 C_1)$$

| Bit Pair ($C_0 C_1$) | Color Output | Visual Transparency |
| :---: | :--- | :--- |
| `00` | Transparent | Pass-through to background or underlying plane. |
| `01` | Sprite Extra Color 1 (`$D025`) | Shared across all multi-color sprites. |
| `10` | Sprite Main Color (`$D027 + i`) | Unique to the active sprite index. |
| `11` | Sprite Extra Color 2 (`$D026`) | Shared across all multi-color sprites. |

### C/Vulkan Decoding Routine:
```c
// Extract multi-color enable mask
uint64_t multicolor_enable = get_register_val(g_graphics_address, 53276);
bool is_multicolor = (multicolor_enable & (1 << sprite_idx)) != 0;

uint32_t extra_color_1 = c64_palette[get_register_val(g_graphics_address, 53285) & 0xF];
uint32_t extra_color_2 = c64_palette[get_register_val(g_graphics_address, 53286) & 0xF];
uint32_t main_color = c64_palette[get_register_val(g_graphics_address, 53287 + sprite_idx) & 0xF];

for (int r = 0; r < 21; r++) {
    uint32_t row_val = pattern[r];
    if (is_multicolor) {
        // Render 12 double-width pixels
        for (int c = 0; c < 12; c++) {
            // Extract 2-bit color code from left to right
            uint8_t bit_pair = (row_val >> (22 - c * 2)) & 0x03;
            uint32_t pixel_color = 0;
            
            switch (bit_pair) {
                case 0x01: pixel_color = extra_color_1; break;
                case 0x02: pixel_color = main_color;    break;
                case 0x03: pixel_color = extra_color_2; break;
                default:   continue; // 00 is transparent
            }
            
            // Plot double-width pixel on staging buffer
            plot_pixel(sb, x + c * 2,     y + r, pixel_color);
            plot_pixel(sb, x + c * 2 + 1, y + r, pixel_color);
        }
    } else {
        // Standard single-color mode: 24 pixels
        for (int c = 0; c < 24; c++) {
            if ((row_val & (1 << (23 - c))) != 0) {
                plot_pixel(sb, x + c, y + r, main_color);
            }
        }
    }
}
```

---

## 3. Rendering Pipeline in Vulkan

The pixel plotting maps the decoded RGB values into the `StagingBuffer` (`sb->data`), which is copied to the Vulkan staging textures via `vkCmdPipelineBarrier` and uploaded to the GPU VRAM:

1. **Host-Side Blitting**: The CPU performs highly optimized bit-pair extraction (optionally vectorized using AVX-512 in `tsfi_opt_zmm.c` for multiple sprites simultaneously).
2. **GPU Texture Upload**: The updated staging buffer is synced to the Vulkan frame memory.
3. **Pipeline Draw Call**: A final quad shader renders the composite buffer, applying scanline offsets and retro CRT bloom shaders to produce vintage phosphor screen outputs.
