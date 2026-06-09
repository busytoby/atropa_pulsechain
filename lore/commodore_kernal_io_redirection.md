# Commodore KERNAL ROM I/O Redirection & Device Abstraction

Long before Unix streams became the industry standard, Commodore's engineers designed a highly advanced **device-independent I/O channel abstraction** inside the PET Kernal (which was subsequently inherited by the VIC-20 and Commodore 64). 

This system allowed software to read from or write to any physical device—keyboard, screen, cassette tape, disk drive, or printer—using the exact same input/output routines.

This analysis details the low-level registry tables, vector offsets, and execution loops of the KERNAL's channel routing architecture, accompanied by a Yul-based stream router emulation.

---

## 1. The Channel Vector Table & Device ID System

The KERNAL assigns a logical **Device ID (0–31)** to every hardware interface:
* **Device 0:** Keyboard (Input only)
* **Device 1:** Datasette Cassette Tape (Input/Output sequential)
* **Device 2:** RS-232 User Port (Serial communications)
* **Device 3:** VIC-II Screen (Output only; maps to screen text RAM `$0400–$07E7`)
* **Device 4–5:** Serial Printers
* **Device 8–11:** Serial Disk Drives (1541, 1571, 1581)

### 1.1 The Active Channel Registers
The KERNAL tracks active channels via zero-page system variables:
* **`$99` (Active Input Device):** Holds the ID of the current source device (set by `CHKIN`).
* **`$9A` (Active Output Device):** Holds the ID of the current destination device (set by `CKOUT`).

### 1.2 Core Vector Table
User programs interact with channels via standard jump vectors at the very top of memory (`$FFC0–$FFF3`):

| Vector Address | Name | Description |
| :--- | :--- | :--- |
| **`$FFC0`** | **`OPEN`** | Opens a logical file linked to a physical device and secondary address. |
| **`$FFC3`** | **`CLOSE`** | Closes the logical file channel. |
| **`$FFC6`** | **`CHKIN`** | Directs the KERNAL to pull all future inputs from this channel (sets zero-page `$99`). |
| **`$FFC9`** | **`CKOUT`** | Directs the KERNAL to push all future outputs to this channel (sets zero-page `$9A`). |
| **`$FFCC`** | **`CLRCHN`**| Restores default input (keyboard/0) and output (screen/3) channels. |
| **`$FFCF`** | **`CHRIN`** | Gets a byte from the active input channel (reads `$99`). |
| **`$FFD2`** | **`CHROUT`**| Writes a byte to the active output channel (reads `$9A`). |

---

## 2. Low-Level Execution Hook: Stream Redirect Simulation

When a program calls `CHROUT` (`$FFD2`), the KERNAL performs a jump table lookup based on the active output device ID (`$9A`):

```
                       [Program calls CHROUT ($FFD2)]
                                     │
                     ┌───────────────▼───────────────┐
                     │   Read Active Device ($9A)    │
                     └───────────────┬───────────────┘
                                     │
             ┌───────────────────────┼───────────────────────┐
             ▼                       ▼                       ▼
         Device 3                 Device 8                Device 4
         [Screen]                 [Disk]                  [Printer]
     Write to Text RAM      Shift out to IEC Bus      Strobe parallel bits
```

### 2.1 The Redirection Hook Loop in Yul
Below is a Yul function simulating the KERNAL ROM's channel router. It handles active input/output registrations, routes writes to simulated screen buffers or disk command channels, and enforces active device flags.

```yul
// Method 22: simulateKernalIo(opCode, val, logicalFileId, deviceId, secondaryAddress)
// Selector: 0xa9c381f2
if eq(selector, 0xa9c381f2) {
    let opCode := calldataload(4)
    let val := calldataload(36)
    let fileId := calldataload(68)
    let devId := calldataload(100)
    let secAddr := calldataload(132)

    // Persistent storage simulation slots:
    // Slot 0x400: Active Input Device ID (default 0)
    // Slot 0x420: Active Output Device ID (default 3)
    // Slot 0x440: Logical File Device Mapping Table (16 entries, 32 bytes each)
    
    let activeInput := sload(0x400)
    if iszero(activeInput) { activeInput := 0 } // Keyboard default
    let activeOutput := sload(0x420)
    if iszero(activeOutput) { activeOutput := 3 } // Screen default

    switch opCode
    case 1 { // OPEN (Register File ID to Device Mapping)
        if gt(fileId, 15) { revert(0, 0) }
        // Store mapped device ID and secondary address at Slot 0x440 + fileId * 32
        let fileOffset := add(0x440, mul(fileId, 32))
        sstore(fileOffset, or(shl(8, secAddr), and(devId, 0xff)))
    }
    case 2 { // CHKIN (Set active input channel)
        let fileOffset := add(0x440, mul(fileId, 32))
        let mapping := sload(fileOffset)
        let mappedDev := and(mapping, 0xff)
        sstore(0x400, mappedDev) // Update active input device ID
    }
    case 3 { // CKOUT (Set active output channel)
        let fileOffset := add(0x440, mul(fileId, 32))
        let mapping := sload(fileOffset)
        let mappedDev := and(mapping, 0xff)
        sstore(0x420, mappedDev) // Update active output device ID
    }
    case 4 { // CLRCHN (Restore defaults)
        sstore(0x400, 0) // Keyboard
        sstore(0x420, 3) // Screen
    }
    case 5 { // CHROUT (Write character to active output channel)
        // Route character based on activeOutput device
        switch activeOutput
        case 3 { // Screen: Write to local screen RAM offset
            let screenPos := sload(0x500) // Cursor index
            sstore(add(0x1000, screenPos), val) // Text screen memory space (0x1000)
            sstore(0x500, add(screenPos, 1))
        }
        case 8 { // Disk Drive: Write byte to command buffer (IEC serialization simulation)
            let bufferPos := sload(0x600)
            sstore(add(0x2000, bufferPos), val) // Disk command space (0x2000)
            sstore(0x600, add(bufferPos, 1))
        }
        default {
            revert(0, 0) // Unsupported device write
        }
    }

    mstore(0x00, sload(0x400)) // Current input device
    mstore(0x20, sload(0x420)) // Current output device
    return(0x00, 64)
}
```

---

## 3. Comparison of Kernal Streams vs. Unix Streams

| Feature | Commodore KERNAL ROM | Unix (POSIX) |
| :--- | :--- | :--- |
| **Selection Mechanic** | Logical File Table (`OPEN`/`CHKIN`) | File Descriptors (`open`/`dup2`) |
| **Output Method** | Character Accumulator (`CHROUT`) | Buffered/Unbuffered Block (`write`) |
| **Active Redirection** | Global Zero Page Registers (`$99`/`$9A`) | Thread-Specific Stream Vectors (`stdout`/`stderr`) |
| **Default Fallback** | Key (0) / Screen (3) | `stdin` (0) / `stdout` (1) |
