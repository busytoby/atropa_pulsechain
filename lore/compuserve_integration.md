# CompuServe Integration Blueprint

This document outlines options for integrating vintage CompuServe technologies into our **OMNICOMM Host-Micro Interface (HMI)** terminal emulator.

---

## 1. CompuServe B-Protocol / B+ Protocol File Transfer
* **Description**: A classic packet-based, bidirectional file transfer protocol designed to run over low-baud rate modems with sliding-window packet headers, framing bytes (`DLE`, `SOH`, `ETX`), and CRC-16 validation.
* **Integration Strategy**:
  * Implement a serial packet framer in the PTY connection pipeline.
  * Allow users to download VM screenshots or ROM binary blocks via the terminal shell using native B+ commands.
  
```
+--------------------------------------------------------+
| Frame Start | Type | Seq | Data (0-1024 Bytes) | CRC-16 |
|    DLE SOH  | Byte | Num |    Raw Payload      |  ETX   |
+--------------------------------------------------------+
```

---

## 2. Vidtex RLE Graphics Rasterizer
* **Description**: CompuServe's proprietary Run-Length Encoded vector/raster protocol, used to transmit low-bandwidth weather maps and stock charts to early desktop computers.
* **Integration Strategy**:
  * Map an escape sequence pattern (e.g., `ESC [ ? 4 0 h` or `$D550` MMIO trigger) to divert incoming data to a Vidtex stream decoder.
  * Run-length decodes coordinates and color values to paint pixel columns directly into our terminal's VRAM.

---

## 3. GIF87a/GIF89a LZW Screenshot Dumper
* **Description**: The Graphics Interchange Format (GIF) was created by CompuServe in 1987.
* **Integration Strategy**:
  * Replace the basic screenshot output with a custom LZW-compressed GIF encoder.
  * Encode terminal frame buffers or sprite grids into a standards-compliant `.gif` file to instantly view VM states in any modern browser.

---

### Recommended Next Steps
1. **Option A (Recommended)**: Build the **GIF87a Screenshot Encoder** to allow exporting terminal frames directly as compressed GIF images.
2. **Option B**: Add **Vidtex RLE Decoders** to allow rendering of classic graphical streams directly inside the terminal window.
3. **Option C**: Implement the **B+ Protocol** packet parser to facilitate raw file transfers.
