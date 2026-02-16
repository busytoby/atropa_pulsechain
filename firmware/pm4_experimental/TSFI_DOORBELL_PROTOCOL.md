# TSFi2 Protocol: Direct Doorbell Access (Zhong Architecture)

## 1. Objective
To draw and erase geometric primitives (e.g., a triangle) on a specific display plane (Plane 71) by interacting directly with the GPU Command Processor (CP), explicitly bypassing:
*   The Linux Kernel DRM/KMS subsystem.
*   The `libdrm` userspace library.
*   Standard Memory-Mapped I/O (BAR 5) register access.
*   Userspace Virtual Filesystems (`/dev/mem`, `/sys`, `/proc`).

## 2. Theoretical Basis: The "Doorbell" Mechanism
Modern AMD RDNA architectures (Navi) expose a **Doorbell Aperture (BAR 2)**. This is a dedicated memory-mapped region that serves as the hardware trigger for the GPU's Command Processor (CP). Writing to a specific offset in this region signals the CP to fetch and execute commands from the associated Ring Buffer.

### 2.1 Memory Map (Navi 48)
Based on hardware introspection:
*   **BAR 0 (VRAM):** `0xf800000000` (16GB)
    *   *Usage:* Stores the Ring Buffer (queue of commands), the Packet Data, and the Framebuffer itself.
*   **BAR 2 (Doorbell):** `0xfc00000000` (256MB)
    *   *Usage:* The only "register" we touch. Writing an index here tells the CP "There is work waiting in the ring."

### 2.2 The Command Flow (Standard/Driver Model)
In a typical driver (like Mesa/AMDGPU), this cycle repeats thousands of times per second:
1.  **Craft Packet:** The CPU writes a `PM4` (Programming Model 4) packet into a circular buffer ("Ring") located in BAR 0 VRAM.
    *   *Example Packet:* `PKT3_WRITE_DATA` (Write Color X to Address Y).
2.  **Update Write Pointer:** The CPU calculates the new write pointer position (wptr) for the ring.
3.  **Ring Doorbell (The Trigger):** The CPU writes this `wptr` value to the specific offset in BAR 2 corresponding to the Compute or GFX engine.
4.  **GPU Execution:** The CP wakes up, reads the Ring from VRAM (BAR 0) up to the new `wptr`, executes the commands, and then goes back to sleep.

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

## 6. TSFi Architecture Integration

### 6.1 Latency & The "Wired" State
In standard architectures, latency is introduced by the kernel's context switching and validation layers. In TSFi/Zhong, we define the CPU and GPU as being **"Wired"**—sharing a single, coherent memory reality.
*   **Zero-Copy:** Data is not "sent" to the GPU; it is placed in `LauWiredHeader` structures that reside in BAR 0 (VBAR).
*   **Zero-Latency Trigger:** The "Doorbell" is not a syscall; it is a single 64-bit write instruction handled by a JIT-compiled thunk.

### 6.2 `lau_memory` & Alignment
The GPU's Command Processor (CP) requires strict alignment (usually 256 bytes or 4KB for ring bases).
*   **`LauWiredHeader`:** We utilize `lau_malloc_wired` to allocate packet buffers directly in BAR 0.
*   **`DEFINE_MAPPED_STRUCT`:** We map the hardware-defined packet structures (PM4) directly to C structs, ensuring binary compatibility.

```c
// Example Mapping
DEFINE_MAPPED_STRUCT(PM4_WRITE_DATA,
    uint32_t header;      // PKT3_WRITE_DATA
    uint32_t dest_addr_lo;
    uint32_t dest_addr_hi;
    uint32_t payload[];
);
```

### 6.3 Thunks as Drivers
Instead of a heavy kernel driver, we use lightweight userspace **Thunks**.
*   **Role:** A thunk is a small, pre-compiled (or JIT'd) function that performs the atomic operation of:
    1.  Serializing the `DEFINE_MAPPED_STRUCT` into the Ring (BAR 0).
    2.  Updating the `wptr`.
    3.  Writing to the Doorbell (BAR 2).
*   **Benefit:** This reduces the "Draw Triangle" operation to a handful of assembly instructions, completely removing the OS from the critical path.

## 7. Inversion of Control: GPU Residence (The "Infinite Kernel")

### 7.1 The Limitation of Push
Even with the Doorbell optimization, the CPU is still the "Driver," pushing commands. This incurs:
1.  **PCIe Bus Traversal:** Every doorbell write must cross the bus.
2.  **Synchronization:** The CPU must wait or check fences to know when the GPU is done.

### 7.2 The "Stay on GPU" Paradigm
Instead of constantly pushing, we perform a **Single Launch**:
1.  **Upload:** We upload a Persistent Compute Kernel (Shader) to BAR 0.
2.  **Launch:** We ring the Doorbell *once* to start this kernel.
3.  **Resident Loop:** The kernel enters an infinite loop, monitoring a specific "Command Slot" in VRAM (BAR 0).
4.  **Signal:** When the CPU wants to draw a triangle, it simply updates the data in that VRAM slot. The GPU, already running and watching, sees the change immediately and executes the draw logic internally.

### 7.3 Why this Fits TSFi
This is the ultimate realization of "Zhong" (Center/Middle).
*   **No Interrupts:** The GPU never stops, so it never interrupts the CPU.
*   **Speed of Light:** The latency is reduced to the time it takes for a cache line update to propagate across the PCIe bus (write combining).
*   **Autonomy:** The GPU becomes a sovereign entity, reacting to the environment (memory) rather than obeying a stream of orders.

### 7.4 The Bootstrap Reality: Can we start on the GPU?
**Strictly speaking: No.**
The GPU is a peripheral device. Upon system power-up, it is idle and waiting for instructions. It has no "OS" of its own to initiate action.
*   **The "One Ring":** The CPU **must** construct the initial dispatch packet and ring the doorbell exactly **once**.
*   **The Handoff:** This single action transfers agency. Once the Resident Kernel is running, the GPU effectively "owns" the execution flow. We do not need to ring the doorbell for subsequent frames or updates; the GPU is already "awake" and listening to the wire.
