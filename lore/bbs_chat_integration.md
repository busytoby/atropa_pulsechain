# QING, GWAT, and Void Chat Integration with the BBS

This document details the architectural layout for bridging the existing **QING/GWAT** and **Void Chat** systems with the on-chain Commodore 64 BBS. By utilizing EVM log monitoring and mapping chat state to virtual C64 I/O addresses, we create a unified, multi-user messaging bridge.

---

## 1. The On-Chain Event Model

The QING and GWAT contracts broadcast messages using standard Solidity events:
```solidity
event LogEvent(string username, uint64 soul, uint64 aura, string msg);
```
Void Chat operates similarly, outputting raw message strings to the transaction logs. To integrate these with our BBS, we construct a bi-directional bridge.

---

## 2. Inbound Bridge: Events to C64 Screen RAM

To display QING/GWAT logs directly on the emulated C64 BBS screen:

1. **Host-Level Event Daemon**:
   A background service (or the browser-side emulator) listens for the `LogEvent` topic hash (`0xbc8...`).
2. **Buffer Injection**:
   When a log is detected, the daemon translates the UTF-8 payload to PETSCII characters.
3. **Memory Mapping ($D5D0–$D5DF)**:
   The daemon injects the parsed fields (`username`, `msg`) into the keyboard queue buffer or a dedicated chat register window at `$D5F0` on the emulated C602 CPU.
4. **IRQ Notification**:
   The virtual hardware triggers an `IRQ` interrupt on the active users' CPUs, signaling that a new chat packet is ready to be pulled and printed to screen RAM (`$0400`).

```
[QING / GWAT / Void Chat Contracts]
             │ (emits LogEvent)
             ▼
      [EVM Logs (Topic 0)]
             │ (monitored by daemon)
             ▼
    [PETSCII Translation]
             │ (injects into emulator)
             ▼
   [C64 Virtual Memory $D5F0] ──► [Triggers IRQ] ──► [Prints to C64 Screen]
```

---

## 3. Outbound Bridge: BBS Terminal to QING/GWAT

To send chat messages from the BBS terminal back out to the global QING/GWAT stream:

1. **Command Input**:
   When a user types `/post <msg>` in the BBS terminal, the C64 CPU buffers the text.
2. **Trigger Register ($D5F8)**:
   The CPU writes the text pointer and length to the outbound chat registers `$D5F8`–`$D5FA`.
3. **EVM Transaction Dispatch**:
   Writing to the trigger register halts emulator execution temporarily. The emulator extracts the string from memory, calls the contract's chat interface (e.g., executing `Chat(string)` or `React(...)`), and continues execution once the transaction is sent or simulated.

---

## 4. Shared "Void Room" Space

We can dedicate a page of our **Global Shared Memory** (`$C100`–`$C1FF`) as a scrollable chat log. 
* Any user poking bytes to this page instantly updates the screen for all other users.
* A cron/worker routine can sync the latest 10 messages from the QING event history into this memory page at startup, ensuring players immediately see the global chat history when they boot their virtual systems and connect to the BBS.

---

## 5. 6502 Assembly Interface for BBS Users

To monitor and participate in the chats directly from the virtual C64 terminal, users execute programs that interact with the mapped registers:

### 1. Chat Monitor (Polling and Printing)
This routine runs continuously on the C64 CPU, checking if a new chat character has arrived in the buffer, and printing it to the terminal screen:

```assembly
; Mapped Registers:
; $D5F0: Buffer Status (0 = empty, >0 = bytes available)
; $D5F1: Read byte from chat event queue

monitor_loop:
    LDA $D5F0         ; Check if chat bytes are available
    BEQ monitor_loop  ; Loop if empty
    
read_char:
    LDA $D5F1         ; Load character from queue
    JSR $FFD2         ; Call KERNAL BSOUT (print char to screen)
    JMP monitor_loop  ; Return to monitoring
```

### 2. Chat Participation (Keyboard Input to EVM)
This routine reads user keyboard input and transmits the completed message string out to the EVM network when the user hits Return:

```assembly
; Mapped Registers:
; $D5F8: Outbound character write register
; $D5F9: Send trigger (write 1 to execute EVM transaction)

input_msg:
    LDX #$00          ; Clear buffer index
input_loop:
    JSR $FFE4         ; Call KERNAL GETIN (get key from buffer)
    BEQ input_loop    ; Loop if no key pressed
    CMP #$0D          ; Check if Return key (ASCII 13)
    BEQ send_message  ; If Return, go to transmission
    
    ; Echo character on screen and store
    JSR $FFD2         ; Print character to screen
    STA $D5F8         ; Poke byte directly to outbound EVM register
    INX
    CPX #$50          ; Max limit (80 characters)
    BNE input_loop

send_message:
    LDA #$01
    STA $D5F9         ; Write 1 to trigger outbound EVM transaction
    JSR $FFD2         ; Print newline (echo Return)
    RTS
```
