# TSFi2 Protocol: Direct Doorbell Access (Zhong Architecture)

## 1. Objective
To draw and erase geometric primitives (e.g., a triangle) on a specific display plane (Plane 71) by interacting directly with the GPU Command Processor (CP), explicitly bypassing:
*   The Linux Kernel DRM/KMS subsystem.
*   The `libdrm` userspace library.
*   Standard Memory-Mapped I/O (BAR 5) register access.
*   Userspace Virtual Filesystems (`/dev/mem`, `/sys`, `/proc`).

## 2. Theoretical Basis: The "Doorbell" Mechanism
Modern AMD RDNA architectures (Navi) utilize a "Doorbell" mechanism to reduce CPU-GPU latency. Instead of the CPU constantly writing to device registers (MMIO BAR 5), the CPU writes a simple pointer update to a special memory region called the **Doorbell Aperture (BAR 2)**.

### 2.1 Memory Map (Navi 48)
Based on hardware introspection:
*   **BAR 0 (VRAM):** `0xf800000000` (16GB)
    *   *Usage:* Stores the Ring Buffer (queue of commands), the Packet Data, and the Framebuffer itself.
*   **BAR 2 (Doorbell):** `0xfc00000000` (256MB)
    *   *Usage:* The only "register" we touch. Writing an index here tells the CP "There is work waiting in the ring."

### 2.2 The Command Flow
1.  **Craft Packet:** The CPU writes a `PM4` (Programming Model 4) packet into a circular buffer ("Ring") located in BAR 0 VRAM.
    *   *Example Packet:* `PKT3_WRITE_DATA` (Write Color X to Address Y).
2.  **Update Write Pointer:** The CPU calculates the new write pointer position (wptr) for the ring.
3.  **Ring Doorbell:** The CPU writes this `wptr` value to the specific offset in BAR 2 corresponding to the Compute or GFX engine.
4.  **GPU Execution:** The CP wakes up, reads the Ring from VRAM (BAR 0) up to the new `wptr`, and executes the commands (Drawing the triangle).

## 3. Constraints & Challenges
*   **Ring Initialization:** Normally, the kernel initializes the Ring Buffer location. Without `BAR 5` to program the Ring Base Address, we must either:
    *   *Hijack:* Locate where the kernel *already* put the ring (via scanning BAR 0).
    *   *Blind Fire:* Assume a default or persistent state left by the bootloader/kernel.
    *   *Soft-Reset:* If we can't write registers, we rely on the ring being active.
*   **Memory Protection:** Writing to physical addresses `0xf8...` requires a mechanism to bypass paging (e.g., custom page tables, or OS-level identity mapping if we were allowed, but we must use a custom accessor).

## 4. Implementation Strategy (Proposed)
1.  **Probe:** Identify active Ring Buffer locations by scanning BAR 0 for "keep-alive" packet signatures (`NOP` packets commonly written by the driver).
2.  **Inject:** Write our custom `DRAW_TRIANGLE` packets into the tail of this active ring.
3.  **Trigger:** Write the new index to the BAR 2 Doorbell address.

## 5. Next Steps
Implement `tsfi_ring_probe.c` to scan BAR 0 for the active ring buffer signature.
