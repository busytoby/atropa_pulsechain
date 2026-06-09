# EasyChain 64: EVM-Compatible C64 Terminal Design Specification

**EasyChain 64** is a conceptual terminal application designed to run on the Commodore 64 (either on real hardware with a serial interface or within our emulated EVM environment) to monitor, interact with, and sign transactions on Ethereum-compatible blockchains.

---

## 1. Technical Architecture Overview

To allow an 8-bit computer with 64KB RAM to communicate with a 256-bit EVM without overwhelming the 6502 CPU with cryptography and JSON parsing, EasyChain 64 uses a **Gateway Proxy** pattern:

```
┌─────────────────┐           ┌──────────────────┐           ┌──────────────────┐
│  Commodore 64   │  Serial   │   Modem Bridge / │ JSON-RPC │   EVM Node /     │
│  EasyChain 64   │◄─────────►│  Browser Daemon  │◄────────►│  Smart Contracts │
│ (6502 Terminal) │  (ACIA)   │  (Metamask/Node) │          │  (BBS/QING/GWAT) │
└─────────────────┘           └──────────────────┘           └──────────────────┘
```

1. **Terminal Frontend (C64)**: Written in 6502 assembly, responsible for rendering terminal text (PETSCII), scanning keyboards, and writing data packets to the ACIA serial interface (at `$D700`).
2. **Modem Bridge (Host/Gateway)**: A local proxy daemon (or browser emulator script) that listens to the C64's serial port. It translates simple serial commands (like AT commands) into rich JSON-RPC calls, handles transaction signing via a private key or Metamask, and returns clean, byte-serialized responses back to the C64.

---

## 2. Serial Communication Protocol (AT-Chain Commands)

EasyChain 64 uses a specialized command set extending standard Hayes "AT" modem commands:

* **`ATB` (Block Number)**: Queries the current block number.
  * *Input*: `ATB`
  * *Response*: `BLOCK: 13495\r\nOK\r\n`
* **`ATC <address>` (Call View Function)**: Queries a contract view method.
  * *Input*: `ATC 0x8C2C... balanceOf(0xf39F...)`
  * *Response*: `VAL: 120000000000000000000\r\nOK\r\n`
* **`ATS <to_address> <calldata>` (Send Transaction)**: Triggers a state-changing transaction.
  * *Input*: `ATS 0x96B4... 0x9812a4df...`
  * *Bridge Action*: Prompts the user to approve and sign the transaction, broadcasts it, and waits for confirmation.
  * *Response*: `TXHASH: 0x1154... STATUS: SUCCESS\r\nOK\r\n`

---

## 3. Terminal Client Code (6502 Assembly Interface)

Below is the design for the core connection loop inside the **EasyChain 64** client:

```assembly
; EasyChain 64 Client Interface
; Registers:
; $D700: ACIA Control Register
; $D701: ACIA Status Register
; $D702: ACIA Data Receive/Transmit

init_terminal:
    LDA #$09         ; Set speed (9600 baud equivalent)
    STA $D700
    RTS

send_command:
    ; Send "ATB" command to get current block
    LDX #$00
cmd_loop:
    LDA atb_cmd,X
    BEQ wait_response
    STA $D702        ; Transmit character
    INX
    JMP cmd_loop

wait_response:
    LDA $D701        ; Read Status Register
    AND #$01         ; Check if byte received
    BEQ wait_response
    LDA $D702        ; Read received character
    CMP #$0D         ; If Carriage Return, finish line
    BEQ render_done
    JSR $FFD2        ; Print character to terminal screen
    JMP wait_response

render_done:
    RTS

atb_cmd:
    .text "ATB"
    .byte 13, 0      ; CR, null-terminator
```

---

## 4. On-Chain Gateway Support (Decentralized Indexers)

To prevent the C64 from executing complex logs/event queries, the EVM contracts host **Helper/Registry Contracts**.
Instead of parsing 100 log events to build a list of active chat participants, the C64 sends `ATC <helper_address> getActiveUsers()`. The helper contract aggregates the data into a single 32-byte memory segment containing a compact list, which the C64 reads directly.
