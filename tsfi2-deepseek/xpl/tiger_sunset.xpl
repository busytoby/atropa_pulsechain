/* XplOS Tiger Sunset Executable JCL Card */

/* Initialize Coaxial RAU Environment */
INIT_RAU 42

/* Set graphics mode (REG_GFX_MODE = 2 for Vulkan Wayland) */
WRITE_ABD 2 1 1 1 0

/* Set screen width to 40 */
WRITE_ABD 40 1 1 1 0

/* Set screen height to 12 */
WRITE_ABD 12 1 1 1 0

/* Set frame buffer pointer address to 0x0400 */
WRITE_ABD 1024 1 1 1 0

/* Draw Sun ('O') at VRAM coordinate center (offset 1224) */
WRITE_ABD 79 1 1 1 0

/* Draw Tiger Silhouette ('^') at VRAM offset (offset 1380) */
WRITE_ABD 94 1 1 1 0

/* Draw Horizon ('=') at VRAM coordinates (offset 1400) */
WRITE_ABD 61 1 1 1 0
