# Comstar T/F Printer: On-Chain Emulation & Escape Sequence translation

This document outlines a design proposal to integrate emulation support for the popular **Comstar T/F (Tractor/Friction) dot-matrix printers** alongside our existing MW-302 parallel interface.

---

## 1. Historical Context

In the mid-1980s, Commodore users looking to avoid the high cost of official Commodore brand printers (like the MPS 801 or 1526) frequently bought third-party alternatives. **Comstar** (advertised heavily in retro magazines like *RUN* and *COMPUTE!*) offered affordable tractor/friction printers.

However, standard Commodore computers output text in **PETSCII** and communicate over the proprietary serial IEEE-488 bus, while the Comstar printers used standard Centronics parallel connections and expected **ASCII/Standard Escape Codes**. Users required an intelligent adapter (like the Micro World MW-302) to perform the signal and text translation.

---

## 2. Dynamic PETSCII-to-ASCII translation in Yul

To support standard Commodore software sending unmodified PETSCII streams directly to the emulated printer, we will implement an automatic translation table inside the Yul printer strobe logic (`54817`).

### Character Mapping Rules:
- **PETSCII uppercase (`0x41`–`0x5A`)** maps to **ASCII lowercase (`0x61`–`0x7A`)**.
- **PETSCII lowercase (`0x61`–`0x7A`)** maps to **ASCII uppercase (`0x41`–`0x5A`)**.
- **Carriage Return (`0x0D`)** translates to both carriage return and line feed (`0x0D 0x0A`) to prevent overwriting the same line.

### Proposed Yul Translation:
```yul
function translatePetsciiToAscii(c) -> out {
    out := c
    // Handle uppercase rotation
    if and(gte(c, 0x41), lte(c, 0x5A)) {
        out := add(c, 0x20)
        leave
    }
    // Handle lowercase rotation
    if and(gte(c, 0x61), lte(c, 0x7A)) {
        out := sub(c, 0x20)
        leave
    }
}
```

---

## 3. Emulating Comstar Escape Codes (Formatting)

Comstar printers supported standard dot-matrix control codes (often resembling Epson/Esc-P commands). We can parse these control characters inside our on-chain spooler to style the output logs:

| Escape Sequence | Function | Spooler Interpretation |
| :--- | :--- | :--- |
| `ESC + 0x45` (`ESC E`) | Bold Mode On | Wrap subsequent text in HTML `<strong>` tags in the frontend console. |
| `ESC + 0x46` (`ESC F`) | Bold Mode Off | Close bold tags. |
| `ESC + 0x2D + 1` (`ESC -1`) | Underline On | Wrap subsequent text in `<u>` tags in the frontend console. |
| `ESC + 0x2D + 0` (`ESC -0`) | Underline Off | Close underline tags. |

### Event Staging Updates:
When the strobe triggers, the character state machine parses sequences. The emitted `PrintLine` log contains structured rich-text formatting, rendering beautiful formatted printouts inside the frontend doc viewer.
