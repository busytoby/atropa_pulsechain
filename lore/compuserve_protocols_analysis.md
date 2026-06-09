# Deep Dive: CompuServe Terminal Protocols & Subsystems

This document analyzes CompuServe's proprietary protocols and explores how they can be emulated or integrated into our current terminal design.

---

## 1. Vidtex (CompuServe Video Terminal) Escape Sequences
Vidtex terminals supported specific escape sequences for local screen formatting, cursor positioning, and terminal feature negotiation.

| Sequence | Name | Description |
|---|---|---|
| `ESC ESC d` | Clear Screen | Clears the terminal screen and resets cursor to home. |
| `ESC ESC c` | Query Terminal ID | Mainframe requests terminal capabilities (resolutions, color palettes, and local storage). |
| `ESC ESC g` | Graphics Mode | Triggers low-baud RLE vector/raster decoder. |
| `ESC ESC I` | Set Cursor | Position cursor at coordinates (X, Y) encoded as offset bytes. |

---

## 2. CompuServe B-Protocol (B, B+, and Quick-B)
Designed to outperform standard XMODEM over early packet-switched networks (such as Tymnet or CompuServe's own network).

### Key Architectural Advantages:
1. **Sliding Windows**: Allowed sending multiple packets before receiving an acknowledgment (`ACK`), mitigating high packet propagation delays.
2. **Variable Block Sizes**: Dynamic packet size adjustment (from 128 to 1024 bytes) based on line noise.
3. **Robust Resumption**: Supported checkpointing and resuming interrupted file transfers by checking file sizes and offset hashes.
4. **Metadata Exchange**: Packed filenames, creation timestamps, and target permissions directly into the initial handshake block.

---

## 3. Host-Micro Interface (HMI) Mainframe Protocol
HMI was a structural payload system layered over B+ protocol frames, designed for front-end CIM (CompuServe Information Manager) applications:

* **Session Negotiation**: Programmatic login and encryption setup.
* **Menu Serialization**: Transmitting structured navigation menus (tables, button lists, text widgets) in binary format instead of raw terminal ANSI text.
* **Transaction Bundling**: Batching multiple mail reads, forum posts, or catalog requests to execute synchronously on the host mainframe.

---

### Potential Integration Ideas:
* **Option A**: Implement the **Vidtex Cursor Escape Sequence Parser** inside our console rendering pipeline.
* **Option B**: Build a mock **CIS Menu Broker** that emulates navigation shortcuts (like `GO WAGE`, `GO WEATHER`) over our local shell interface.
* **Option C**: Code the **B+ File Transfer sliding-window state machine** to allow native file exchanges with terminal hosts.
