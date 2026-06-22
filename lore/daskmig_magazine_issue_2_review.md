# Daskmig Magazine Issue #2 (November 1994) — Technical & Historical Review

Published in November 1994, the second issue of the MS-DOS diskmag **Daskmig Illusions #2** was created by **Independents Productions** (later rebranded as **iSC**) under the leadership of **Mr. Hockey** (coder) and **Lash** (editor), featuring a soundtrack composed by **Calvin** of Proxima.

---

## 1. Program Structure and Layout

*Daskmig Illusions #2* introduced an improved user interface, moving away from the simple text-fading reader used in the debut issue:

| Component | Target System | Author | Purpose / Architecture |
| :--- | :--- | :--- | :--- |
| **Smooth-Scrolling Reader** | MS-DOS / PC | Mr. Hockey / Independents | A custom VGA-based horizontal/vertical scrolling text engine with scroll bars and page layout optimizations. |
| **Norwegian Scene Articles** | MS-DOS / PC | Independents | Technical columns detailing sound design, programming, and rankings within the Norwegian demoscene. |

---

## 2. Deep Dive: VGA Smooth-Scrolling Engine

*   **VGA Page Flipping**: Mr. Hockey implemented a smooth double-buffer page-flipping mechanism in x86 assembly to prevent screen tearing during page scrolls.
*   **Audio Tracker Integration**: The reader integrated low-level Sound Blaster DMA playback routines to play tracker music (compiled by Calvin) concurrently with input reading.
*   **Custom Interface Widgets**: Included primitive mouse-coordinate polling and graphical scroll bars on top of standard VGA character grids.

---

## 3. Emulation Integration Path

To integrate these utilities into our virtual environment:
1.  **Scrolling Physics Simulation**: The horizontal and vertical scroll wrap mechanics are mapped to our canvas display presenting buffers to verify continuous frame offsets.
2.  **Sound Card Polling**: Audio tracker triggers are mapped to our resident ALSA synthesizer to simulate historical sound hardware interrupt sweeps.
