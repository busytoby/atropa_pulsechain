# ANTIC display List: Benefits for ZMM VM Renderer & Terminal

Integrating an **ANTIC-like Display List** processor design in Yul offers architectural advantages for the **ZMM Virtual Machine** and its terminal emulators (such as the Wayland interactive terminal shell in our repository). 

This analysis details how decoupling video rendering from the CPU using display list instructions optimizes rendering speed, memory overhead, and terminal viewport control.

---

## 1. Core Architectural Advantages for ZMM

```
  ┌────────────────────────────────────────────────────────┐
  │                       ZMM CPU                          │
  │   Writes simple instructions to Display List buffer    │
  └──────────────────────────┬─────────────────────────────┘
                             │
  ┌──────────────────────────▼─────────────────────────────┐
  │                    Yul ANTIC Engine                    │
  │   Parses Display List instructions via memory offsets  │
  └──────────────────────────┬─────────────────────────────┘
                             │
  ┌──────────────────────────▼─────────────────────────────┐
  │                     ZMM Renderer                       │
  │   Renders scanlines based on current mode & offsets   │
  └────────────────────────────────────────────────────────┘
```

### 1.1 Zero-Copy Viewport Scrolling
In traditional terminal emulators, scrolling the screen upwards requires shifting all characters in memory (e.g., copying 80 columns $\times$ 24 rows of text, and clearing the bottom row). 
* **The ANTIC Solution:** By using ANTIC's **Load Memory Scan (LMS)** instruction, we can scroll the viewport by changing a single 16-bit pointer in the display list. The renderer shifts the read offset of screen memory instantly. This reduces scrolling overhead from an $O(N)$ memory copy operation to a strictly **$O(1)$ pointer update**.

### 1.2 Mixed-Resolution Layouts (UI Compartmentalization)
Standard terminal emulators enforce a uniform grid size across the entire window. 
* **The ANTIC Solution:** The display list allows mixing video modes on a row-by-row basis:
  * **Status Bar:** A high-resolution, static 8-pixel tall mode at the top.
  * **Text Console:** Standard 40-column or 80-column monospaced text mode for commands.
  * **Diagnostic Graphs:** High-speed vector bitmap modes mapped mid-screen to display system logs.
  This compartmentalization allows the terminal to display graphs and console output simultaneously without redrawing the entire screen.

### 1.3 Decoupled JIT Compute & Video Refresh
* **The Constraint:** The JIT compiler and simulation engine run at high frequencies. Having the CPU constantly format and write pixels to a frame buffer wastes register files.
* **The ANTIC Solution:** Decoupling the compute state from the video state means the CPU only needs to update character pointers. The Yul-based renderer streams these pointers in the background, minimizing CPU register pollution.

---

## 2. Emulated Register Specification for ZMM Terminal

We can map ANTIC control registers into our ZMM VM address space:

| Register Address | Name | Data Type | Function |
| :---: | :--- | :--- | :--- |
| **`$D400`** | **`DLIST_PTR`** | `uint16` | Pointer to the active Display List program in memory. |
| **`$D402`** | **`HSCROL`** | `uint8` | Horizontal fine scrolling pixel offset (0-7). |
| **`$D403`** | **`VSCROL`** | `uint8` | Vertical fine scrolling pixel offset (0-7). |
| **`$D404`** | **`DMACTL`** | `uint8` | Enable/Disable Direct Memory Access for screen/sprite lists. |
