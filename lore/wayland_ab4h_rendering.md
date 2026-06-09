# Wayland AB4H 64-Bit Presentation with Yul 8-Bit Source Materials

This specification details the design for displaying **64-bit AB4H (`VK_FORMAT_R16G16B16A16_SFLOAT`)** final graphics on a **Wayland window**, while maintaining a lightweight, standard **8-bit memory footprint** on the guest Yul CPU. 

This hybrid pipeline achieves optimal rendering performance: the guest Yul emulation remains simple and compatible with 8-bit CPU instructions, while the Vulkan GPU pipeline performs high-speed color expansion to HDR AB4H format for display.

---

## 1. Pipeline Layout & Memory Mappings

```
  ┌────────────────────────────────────────────────────────┐
  │                   Yul CPU (8-Bit Space)                │
  │   Maintains 8-bit screen character RAM ($0400)          │
  └──────────────────────────┬─────────────────────────────┘
                             │
                             │ Staging DMA Buffer Upload
                             ▼
  ┌────────────────────────────────────────────────────────┐
  │                 Vulkan Texture (8-Bit)                 │
  │   Format: VK_FORMAT_R8_UINT (Pixel Indexes)            │
  └──────────────────────────┬─────────────────────────────┘
                             │
                             │ Shader Expansion via Palette
                             ▼
  ┌────────────────────────────────────────────────────────┐
  │                 Vulkan Swapchain Image                 │
  │   Format: VK_FORMAT_R16G16B16A16_SFLOAT (AB4H 64-Bit)  │
  └──────────────────────────┬─────────────────────────────┘
                             │
                             │ Wayland wl_surface_attach
                             ▼
  ┌────────────────────────────────────────────────────────┐
  │                 Wayland Window Canvas                  │
  │   Displays high-fidelity 64-bit float color graphics   │
  └────────────────────────────────────────────────┘
```

### 1.1 Source Materials (Yul CPU side)
* **Screen Memory Map:** Mapped to address space `$0400–$07E7` (1024 bytes for a $40 \times 25$ character grid).
* **Color RAM:** Mapped to `$D800–$DBE7` (1024 bytes containing 4-bit/8-bit color palette indices).
* **Benefit:** The guest CPU logic writes to these registers using standard 8-bit assembly (e.g. `STA $D800`), requiring no 64-bit operations.

### 1.2 Staging Buffer Upload
1. The emulator reads the 8-bit screen index buffer.
2. The bytes are copied into a GPU-mapped staging buffer (`VkBuffer` allocated via `lau_malloc_wired` with memory property `VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT`).
3. A command buffer records a transfer from the staging buffer to a **`VK_FORMAT_R8_UINT`** texture image using `vkCmdCopyBufferToImage`.

### 1.3 Vulkan Swapchain & Presentation
* **Swapchain Configuration:** The swapchain is created using:
  * `VkSwapchainCreateInfoKHR.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT`
  * `VkSwapchainCreateInfoKHR.imageColorSpace = VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT` (ensuring correct HDR linear blending).
* **Fragment Shader Translation:** The Vulkan pipeline runs a fragment shader that samples the `R8_UINT` texture and writes to the swapchain's `R16G16B16A16_SFLOAT` attachment, translating the 8-bit index to 64-bit AB4H color using a uniform lookup table.

---

## 2. Vulkan Pipeline Configuration (C Code)

Below is the configuration snippet for creating the target Vulkan image view in 64-bit AB4H:

```c
// 1. Configure Swapchain for 64-bit AB4H Floating Point Output
VkSwapchainCreateInfoKHR swapchainInfo = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT,
    .imageColorSpace = VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageArrayLayers = 1,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE
};

// 2. Configure 8-bit Source Index Texture (R8_UINT)
VkImageCreateInfo textureInfo = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
    .imageType = VK_IMAGE_TYPE_2D,
    .format = VK_FORMAT_R8_UINT, // Stores Yul screen RAM indices
    .extent = { 1024, 768, 1 },
    .mipLevels = 1,
    .arrayLayers = 1,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .tiling = VK_IMAGE_TILING_OPTIMAL,
    .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
};
```

---

## 3. Wayland Integration

The swapchain image handle is bound to the Wayland display:
1. **DMA-BUF Sharing:** If using direct scanout, the Vulkan memory backing the `R16G16B16A16_SFLOAT` image is exported as a file descriptor (`dma-buf`).
2. **Wayland Import:** The compositor imports the `dma-buf` using the `zwp_linux_dmabuf_v1` protocol interface, presenting the 64-bit frame directly to the window compositor.
3. **Low-Latency Rendering:** Bypasses software translation, letting the GPU hardware compositor display the 64-bit float surface with zero CPU copy overhead.
