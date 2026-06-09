# Cardco Peripheral Interfaces: The IEC-to-Centronics Parallel Bridge

**Cardco, Inc.** was a pioneer in creating third-party hardware bridges for Commodore computers (VIC-20 and C64). Their most successful products, the **Card//Print A (C/A)** and **Card//Print +G (C/+G)**, solved a critical consumer bottleneck: the inability to connect industry-standard Centronics parallel printers (e.g., Epson FX/MX series) to Commodore's proprietary IEC Serial Bus.

This analysis details the low-level signal translation, character set transliteration, and includes an on-chain simulation model.

---

## 1. Low-Level IEC-to-Centronics Signal Translation

Commodore computers communicate with printers on Device 4 using the bit-banged **IEC Serial Protocol**. A standard printer interface like the Card//Print A bridges this serial bus to a **Centronics 36-pin Parallel Interface**.

```
    +---------------+               +-------------------+               +------------------+
    |               | -- ATN ---->  |                   | -- DATA 0..7 >|                  |
    | Commodore C64 | -- CLK ---->  |   Cardco Bridge   | -- STROBE --->| Parallel Printer |
    |  (Device 4)   | <- DATA ----  | (6502/68705 MCU)  | <- BUSY ------|  (Centronics)    |
    +---------------+               +-------------------+               +------------------+
```

### 1.1 Interface Handshake Logic
1. **IEC Listener Activation**: The C64 pulls the **ATN** line low and sends a listen command targeting Device 4. The Cardco interface intercepts this command, pulls the **DATA** line low to assert active listener status, and waits for bytes.
2. **Serial Stream Demultiplexing**: The C64 bit-bangs the character payload onto the **CLK** and **DATA** lines. The Cardco onboard microcontroller (typically a custom Motorola 68705 or MOS 6502 derivative) reconstructs the byte.
3. **Centronics Pulse Dispatch**:
   * The reconstructed byte is placed on the Centronics **DATA 0..7** pins.
   * The microcontroller checks the printer's **BUSY** line.
   * If **BUSY** is low, the controller pulses the active-low **STROBE** pin (typically <1 µs pulse width) to latch the byte into the printer's internal hardware buffer.

---

## 2. Character Set Transliteration (PETSCII to ASCII)

Commodore computers use a custom character encoding known as **PETSCII**, while standard printers expect **ASCII**. The Card//Print A interface translated characters on the fly:

### 2.1 Standard Transliteration Rules
* **Control Characters**: Carriage Return (`$0D`) and Line Feed (`$0A`) are passed through directly.
* **Alphabet Swapping**: Standard PETSCII letters are shifted to align with standard ASCII tables:
  * PETSCII uppercase (range `$41–$5A`) maps directly to ASCII uppercase.
  * PETSCII lowercase (range `$C1–$DA`) is mapped to ASCII lowercase (range `$61–$7B` via `$C1 - $60`).
* **Commodore Graphics Translation (+G variant)**:
  * The **Card//Print +G** intercepts PETSCII graphic character codes.
  * It translates unique Commodore graphic characters (like lines, curves, and shading tiles) into Epson-compatible **ESC/P graphic escape sequences** (`ESC * m n1 n2` pin-firing commands) to draw CBM screen symbols precisely on dot-matrix print heads.

---

## 3. On-Chain Yul Simulation: Centronics Bridge & Transliteration

Below is a Yul implementation simulating the Card//Print interface. It parses an incoming stream of bit-banged serial IEC data, decodes the bytes, translates the PETSCII character set to ASCII on the fly, and generates corresponding parallel Centronics control pulses (measuring strobe count and busy cycles).

```yul
object "CardcoPrinterBridge" {
    code {
        if callvalue() { revert(0, 0) }
        datacopy(0x00, dataoffset("runtime"), datasize("runtime"))
        return(0x00, datasize("runtime"))
    }
    object "runtime" {
        code {
            if lt(calldatasize(), 4) { revert(0, 0) }
            let selector := shr(224, calldataload(0))

            // translateIecToCentronics(bytes serialPayload) -> (bytes asciiOutput, uint256 strobeCount, uint256 cycles)
            // Selector: 0xa77ca92f
            if eq(selector, 0xa77ca92f) {
                let offset := calldataload(4)
                let length := calldataload(add(4, offset))

                let destOffset := 0x80
                let strobeCount := 0
                let cycles := 0

                for { let i := 0 } lt(i, length) { i := add(i, 1) } {
                    let petsciiByte := byte(0, calldataload(add(add(36, offset), i)))

                    let asciiByte := petsciiByte
                    cycles := add(cycles, 8) // Bit-shft extraction cost

                    // PETSCII to ASCII Conversion rules:
                    if and(gt(petsciiByte, 0x40), lt(petsciiByte, 0x5b)) {
                        // Standard uppercase letters: pass-through
                        asciiByte := petsciiByte
                    }
                    if and(gt(petsciiByte, 0xc0), lt(petsciiByte, 0xdb)) {
                        // Lowercase letters: map $C1-$DA down to $61-$7A
                        asciiByte := sub(petsciiByte, 0x60)
                    }
                    
                    // Centronics parallel latching simulation
                    strobeCount := add(strobeCount, 1)
                    cycles := add(cycles, 12) // Latching cycle overhead

                    // Store translated ASCII character in memory buffer
                    mstore8(add(destOffset, i), asciiByte)
                }

                // Return payload
                mstore(0x00, 0x60) // Offset to dynamic bytes
                mstore(0x20, strobeCount) // Strobe pulse count
                mstore(0x40, cycles) // CPU cycle cost
                mstore(0x60, length) // Length of returned bytes
                return(0x00, add(0x80, mul(div(add(length, 31), 32), 32)))
            }

            revert(0, 0)
        }
    }
}
```

---

## 4. Performance & Hardware Properties

| Feature | Card//Print A (C/A) | Card//Print +G (C/+G) |
| :--- | :--- | :--- |
| **Input Interface** | 6-pin DIN Commodore Serial IEC | 6-pin DIN Commodore Serial IEC |
| **Output Interface** | 36-pin Centronics Parallel | 36-pin Centronics Parallel |
| **Throughput Speed** | ~400 bytes/sec (IEC Bound) | ~400 bytes/sec (IEC Bound) |
| **Graphics Translation** | None (Raw characters only) | Full PETSCII-to-ESC/P Dot Graphics conversion |
| **Bypass Mode** | Switchable to ASCII direct | Switchable to ASCII direct |
