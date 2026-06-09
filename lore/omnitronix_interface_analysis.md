# Omnitronix Deluxe User Port Serial Interface — Technical Analysis

In the 1980s, **Omnitronix** was a prominent manufacturer of hardware interfaces for the Commodore 64 and VIC-20. Their flagship hardware was the **Deluxe RS-232 Interface**, which expanded the C64’s connectivity by hooking directly into the 24-pin edge connector of the **User I/O Port**. This allowed the Commodore 64 to interface with standard modems, serial printers, and telecommunications devices.

This document analyzes the hardware layout of the Omnitronix serial system, maps the 6526 CIA port registers, and implements an on-chain RS-232 serial transceiver in Yul.

---

## 1. The C64 User Port & CIA 6526 Map

Unlike standard computers with dedicated UART chips, the Commodore 64 implemented RS-232 serial communication entirely in software by manipulating the registers of the **CIA 2 (Complex Interface Adapter)** chip:

```
  ┌────────────────────────────────────────────────────────┐
  │                   Commodore 64 CPU                     │
  └──────────────────────────┬─────────────────────────────┘
                             │ Reads/Writes CIA Registers
  ┌──────────────────────────▼─────────────────────────────┐
  │                 CIA 2 ($DD00-$DDFF)                    │
  │   - Port B ($DD01) maps to the User Port               │
  │   - Serial TXD/RXD lines mapped to Port B pins         │
  └──────────────────────────┬─────────────────────────────┘
                             │ User Port Edge Connection
  ┌──────────────────────────▼─────────────────────────────┐
  │         Omnitronix Deluxe RS-232 Interface             │
  │   - Level-shifts 0-5V TTL signals to +/-12V RS-232     │
  └────────────────────────────────────────────────────────┘
```

*   **CIA 2 Port B Register (`$DD01` / `56577`)**: Direct read/write access to the User Port pins.
*   **Data Direction Register B (`$DD03` / `56579`)**: Configures each User Port pin as input (`0`) or output (`1`).

### 1.1 Pin-to-Bit Mapping for Omnitronix RS-232
For software-driven RS-232, the Commodore KERNAL maps serial lines to CIA 2 Port B:

| Bit | Pin Name | Description | Direction |
| :---: | :--- | :--- | :--- |
| **0** | **RXD** | Received Data | Input |
| **1** | **TXD** | Transmitted Data | Output |
| **2** | **RTS** | Request To Send | Output |
| **3** | **CTS** | Clear To Send | Input |
| **4** | **DSR** | Data Set Ready | Input |
| **5** | **DCD** | Data Carrier Detect | Input |
| **6** | **DTR** | Data Terminal Ready | Output |
| **7** | **RI** | Ring Indicator | Input |

---

## 2. On-Chain Transceiver: Omnitronix User Port Emulator

To model the Deluxe RS-232 serial interface in Yul, we emulate reading and writing bytes through the `$DD01` Port B memory registers, implementing state transitions for transmitting (TX) and receiving (RX) bytes.

### 2.1 Serial Port Registers (MMIO)
| Memory Address | Name | Data Type | Description |
| :---: | :--- | :--- | :--- |
| **`56577`** | **`CIAPB`** | `uint8` | CIA 2 Port B Data Register (`$DD01`) |
| **`56579`** | **`DDRB`** | `uint8` | CIA 2 Port B Data Direction Register (`$DD03`) |
| **`56600`** | **`TX_BUF`**| `uint8` | Local Transmit Byte Buffer |
| **`56601`** | **`RX_BUF`**| `uint8` | Local Receive Byte Buffer |

### 2.2 Serial Transceiver in Yul
The following Yul code simulates transmitting a byte out of `TX_BUF` by shifting it bit-by-bit into the `TXD` pin (Bit 1) of `CIAPB`, and receiving a byte into `RX_BUF` from the `RXD` pin (Bit 0):

```yul
// Emulates the serial transmission loop of the Deluxe RS-232 Interface
function processSerialTransceiver() {
    let portB := sload(56577) // Read current Port B state
    let ddrB := sload(56579)  // Read Data Direction Register

    // 1. Transmit Loop (Output bit to Pin 1 / TXD)
    // Verify Pin 1 is configured as output in DDRB (bit 1 is set)
    if and(ddrB, 0x02) {
        let txByte := sload(56600) // Load byte to transmit
        
        // Loop through 8 bits to simulate sequential bit transmission
        for { let bitIdx := 0 } lt(bitIdx, 8) { bitIdx := add(bitIdx, 1) } {
            // Get current bit value (0 or 1)
            let bitVal := and(shr(bitIdx, txByte), 1)
            
            // Set or clear Pin 1 (TXD) in Port B data
            if bitVal {
                portB := or(portB, 0x02) // Set Bit 1
            }
            if iszero(bitVal) {
                portB := and(portB, 0xFD) // Clear Bit 1 (mask 11111101)
            }
        }
        
        // Store updated Port B state
        sstore(56577, portB)
    }

    // 2. Receive Loop (Input bit from Pin 0 / RXD)
    // Verify Pin 0 is configured as input in DDRB (bit 0 is clear)
    if iszero(and(ddrB, 0x01)) {
        let rxByte := 0
        
        // Read Port B RXD bit sequentially (represented as 8 clock ticks)
        for { let bitIdx := 0 } lt(bitIdx, 8) { bitIdx := add(bitIdx, 1) } {
            // Read Pin 0 (RXD) state
            let bitVal := and(portB, 0x01)
            
            // Reconstruct the byte
            rxByte := or(rxByte, shl(bitIdx, bitVal))
        }
        
        // Save the assembled byte to Receive Buffer
        sstore(56601, rxByte)
    }
}
```

---

## 3. Comparison with Competitors

| Metric | Omnitronix Deluxe | Cardco Card/Print | Commodore VIC-1011A |
| :--- | :--- | :--- | :--- |
| **Port Hook** | User Port (24-pin edge) | Serial Bus (DIN-6) | User Port (24-pin edge) |
| **Pin Translation** | Standard RS-232 level-shifting | Centronics parallel conversion | Simple 3-line TTL level pass-through |
| **Device Target** | Modems, standard serial printers | Parallel/Centronics printers | Terminals, modems |

---

> [!NOTE]
> The Omnitronix Deluxe interface performed hardware level-shifting, converting the Commodore’s internal 0-5V TTL User Port output to the standard +/-12V signals required by professional RS-232 devices. Without this interface, connecting a modem directly to the C64 User Port risked blowing the CIA 6526 chips due to voltage mismatches.
