# Kilobaud Issue #7: Don Lancaster's TVT-6 Software Raster Scan Video Generator

This document provides a detailed technical analysis of the video hardware concepts presented in *Kilobaud Computing* Issue #7 (July 1977), focusing on Don Lancaster’s TVT-6 raster-scan generator.

---

## 1. Don Lancaster's TVT-6 Video Generator Architecture

In *"Raster Scan Graphics for the 6800 (Part 1: The Hardware)"*, Don Lancaster introduces the **TVT-6 (Television Typewriter)**.

### A. Direct Software Scan Generation
In 1977, video display terminal boards were expensive and complex, requiring dozens of TTL logic chips to generate sync signals, address memory, and shift pixel data.
* **Lancaster's Innovation**: Bypassed hardware controllers by using the microcomputer's main CPU (6800 or 6502) to generate the video raster directly.
* **The Scan Loop**: The CPU enters a tight software loop during the active display time, sequentially reading character memory addresses. The memory output is routed to a shift register that serializes the bits into a composite video signal.
* **Synchronization**: The CPU also manages horizontal and vertical sync timing via software delays or basic interrupts.

### B. Trade-offs
Using the main CPU to scan video meant that **up to 90% of the CPU's processing power** was spent simply keeping the picture stable on the television. The CPU could only perform user computations during the short vertical blanking intervals (when the beam is traveling from the bottom of the screen back to the top).

---

## 2. Application: Virtual TVT-6 Phosphor Dot Raster Plotter

To honor the TVT-6 software raster scanning technique, we can implement a **TVT-6 Raster Render Mode** within our oscilloscope draw routine:
- Instead of tracing clean vector lines (`ctx.lineTo`), the TVT-6 mode draws the signal as a discrete sequence of **glowing phosphor scan dots** aligned with horizontal raster lines.
- Adds slight vertical jitter and scanline decay to replicate the look of a TV screen driven directly by a microprocessor scan loop.
