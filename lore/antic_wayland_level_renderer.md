# ANTIC Implementation at the Wayland/Vulkan Presentation Layer

To achieve maximum rendering performance and eliminate interpreted bytecode emulation cycles inside the guest environment, the **ANTIC Display List interpreter is implemented entirely at the host-side Wayland/Vulkan presentation layer (native C/C++ code and Vulkan shaders)**, rather than executing inside the Yul EVM bytecode.

In this model, the **Yul smart contract serves as a passive register store** (holding the Display List instructions and indices in memory). The **host Vulkan renderer acts as the physical graphics chip**, actively reading the guest registers, executing the Display List program, and translating the 8-bit source data into a high-fidelity 64-bit AB4H Wayland window display.

---

## 1. Hybrid Renderer Architecture

```
  ┌────────────────────────────────────────────────────────┐
  │                   Yul Smart Contract                   │
  │   Stores 8-bit indices & Display List program in RAM   │
  └──────────────────────────┬─────────────────────────────┘
                             │
                             │ GPU Host Reads RAM Registers
                             ▼
  ┌────────────────────────────────────────────────────────┐
  │                 Host C/Vulkan Engine                   │
  │   1. Reads DLIST_PTR ($D400), HSCROL ($D402), etc.     │
  │   2. Parses Display List instructions sequentially     │
  │   3. Generates Vulkan Command Buffers line-by-line     │
  └──────────────────────────┬─────────────────────────────┘
                             │
                             │ Direct Draw Calls with Scissor Offsets
                             ▼
  ┌────────────────────────────────────────────────────────┐
  │                 Vulkan Pipeline (GPU)                  │
  │   Renders mixed modes to 64-bit AB4H Framebuffer       │
  └──────────────────────────┬─────────────────────────────┘
                             │
                             │ Attach Framebuffer
                             ▼
  ┌────────────────────────────────────────────────────────┐
  │                 Wayland Window Display                 │
  └────────────────────────────────────────────────────────┘
```

### 1.1 Passive Register Maps (Yul Space)
The Yul contract simply holds the register state variables in memory:
* **`DLIST_PTR` (`$D400`):** Holds the memory start address of the Display List program.
* **`HSCROL` (`$D402`):** Fine horizontal pixel scroll offset (0-7).
* **`VSCROL` (`$D403`):** Fine vertical pixel scroll offset (0-7).

### 1.2 Host-Side Display List Parser (C Space)
The Vulkan display engine parses the Display List program *before* rendering the frame. It maps the instructions directly to dynamic Vulkan pipelines:

```c
void parse_display_list_to_vulkan(VulkanContext *vk, uint8_t *guest_ram, uint16_t dl_addr) {
    uint16_t pc = dl_addr;
    uint32_t current_scanline = 0;

    VkCommandBuffer cmd = begin_rendering_commands(vk);

    while (current_scanline < 240) {
        uint8_t inst = guest_ram[pc++];
        uint8_t mode = inst & 0x0F;
        bool is_lms = (inst >> 6) & 1;
        bool is_jump = (inst >> 7) & 1;

        if (is_jump) {
            uint16_t target = guest_ram[pc] | (guest_ram[pc+1] << 8);
            pc = target; // Jump to target address
            continue;
        }

        uint16_t pixel_src_addr = 0;
        if (is_lms) {
            pixel_src_addr = guest_ram[pc] | (guest_ram[pc+1] << 8);
            pc += 2;
        }

        // Configure Vulkan dynamic pipeline parameters
        bind_mode_pipeline(cmd, mode);
        set_vulkan_scissor_box(cmd, current_scanline, get_mode_height(mode));
        bind_source_index_texture(cmd, pixel_src_addr);

        current_scanline += get_mode_height(mode);
    }

    end_and_submit_commands(vk, cmd);
}
```

---

## 2. Vulkan Pipeline Configuration & Dynamic Scissoring

To support mixed modes (e.g. text rows alongside hires graphics), the Vulkan pipeline uses **Dynamic Viewports and Scissor States**:

```c
// Enable dynamic states inside pipeline creation
VkDynamicState dynamicStates[] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
};

VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = 2,
    .pDynamicStates = dynamicStates
};
```

During the Display List parse loop, the renderer adjusts the scissor box dynamically for each scanline band:

```c
void set_vulkan_scissor_box(VkCommandBuffer cmd, uint32_t start_y, uint32_t height) {
    VkRect2D scissor = {
        .offset = { 0, (int32_t)start_y },
        .extent = { 1024, height } // Fixed width, variable height
    };
    vkCmdSetScissor(cmd, 0, 1, &scissor);
}
```

---

## 3. Advantages of Wayland-Level Implementation

1. **Zero Interpreter Overhead:** Since ZMM executes EVM bytecode via translation/thunking interfaces, shifting display list loop processing entirely to the host C/Vulkan runtime eliminates interpreted CPU execution cycles, saving significant host hardware processing time.
2. **Hardware-Accelerated Blits:** The Vulkan pipeline performs the index-to-color mapping directly in the fragment shader hardware, outputting native 64-bit float formats (AB4H) at minimal processing cost.
3. **No Redundant Framebuffers:** The CPU only pushes small 8-bit index chunks; the massive 64-bit frame buffer is allocated solely on the GPU VRAM, keeping data transfers lightweight.
4. **Flexible Viewport Control:** Views can be dynamically sized, scaled, and translated on the Wayland window using Vulkan dynamic scissor configurations without altering guest memory coordinates.

