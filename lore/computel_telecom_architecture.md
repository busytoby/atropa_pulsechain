# Auncient Computel Telecommunications Suite: System Architecture

This document details the architectural layout, frequency modulation standards, security mechanisms, and testing frameworks of the **Auncient** Computel Telecommunications simulation suite.

```
                  [User TTY / Controller Input]
                               │
                               ▼
                 [Secure TTY Console / Thunk]
                               │ (XOR Encrypted)
                               ▼
                 [Bell 202 FSK Modulator]
                               │ (1200 Hz / 2200 Hz carrier)
                               ▼
                 [Secured Coaxial Line / UDP Bins] (DMT Bit-Loading)
                               │
                               ▼
                 [Goertzel Demodulator / Decoders]
                               │ (FSK / DTMF / Decadic Pulses)
                               ▼
                 [Yul CPU-Level Firewall]
                               │ (Saat(1) Soul Authentication)
                               ▼
                 [Dysnomia ZMM VM Sandbox] (Helmholtz Command Execution)
```

---

## 1. Physical Layer & Frequency Modulation

### 1.1 Bell 202 FSK Modulator & Demodulator
*   **Standards**: Operates at 1200 Baud, utilizing a sample rate of 8000 Hz.
    *   **Mark (Logical 1)**: $1200\text{ Hz}$ sine wave.
    *   **Space (Logical 0)**: $2200\text{ Hz}$ sine wave.
*   **Encapsulation**: Each byte is serialized using standard UART boundaries: 1 Start Bit (Space), 8 Data Bits (LSB first), and 1 Stop Bit (Mark).
*   **Implementation**: Modulator is located in [tsfi_computel_fsk_modulator.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_computel_fsk_modulator.c) and monitored via [tsfi_computel_carrier_monitor.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_computel_carrier_monitor.c).

### 1.2 Dual-Tone Multi-Frequency (DTMF) Signaling
*   **Standard Keypad Matrix**: Superimposes row and column sine waves:
    *   *Rows (Hz)*: 697, 770, 852, 941
    *   *Columns (Hz)*: 1209, 1336, 1477, 1633
*   **Goertzel Decoding**: Executes frequency-band power calculations over a block window size of $N=205$ samples at 8000 Hz, detecting keypresses with zero library dependencies.
*   **Implementation**: Encoder located in [tsfi_computel_dtmf_encoder.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_computel_dtmf_encoder.c) and decoder in [tsfi_computel_dtmf_decoder.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_computel_dtmf_decoder.c).

### 1.3 Decadic Rotary Pulse Dialing
*   **Loop Disconnect Signaling**: Decodes rotary pulses at $10\text{ pps}$ by parsing Line-State Make/Break sequences.
    *   *Break Pulse*: Loop disconnect ($45\text{ms}$ to $75\text{ms}$ range).
    *   *Inter-Digital Pause*: Minimum $700\text{ms}$ Make to finalize the dialed number.
*   **Implementation**: Located in [tsfi_computel_rotary_decoder.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_computel_rotary_decoder.c).

---

## 2. Command Sequencing & CPU Firewalling

### 2.1 DTMF Sequencer
*   **Command Parsing**: Gathers DTMF keys. Once the termination symbol (`'#'`) is received, the command is processed:
    *   *Trunk Dialing*: Numbers like `800`–`815` dial and bridge lines via [tsfi_computel_trunk_selector.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_computel_trunk_selector.c).
    *   *Overrides*: Key combinations (e.g. `*0`) trigger hypervisor overrides like master resets.
*   **Implementation**: Located in [tsfi_computel_dtmf_sequencer.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_computel_dtmf_sequencer.c).

### 2.2 Yul CPU-Level Firewall
*   **Soul Verification**: Verifies dialed control overrides against the player's authenticated cryptographic `Soul` signature (resolved via on-chain contract registries).
*   **Access Rules**:
    *   *Master Reset (*0)*: Restricts execution to authorized admin `Soul` parameters.
    *   *Carrier Trunk (805)*: Restricts access to whitelist coordinates, blocking unauthenticated network injection.
*   **Implementation**: Verified in [test_computel_yul_firewall.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/tests/test_computel_yul_firewall.c).

---

## 3. Secure TTY & Cryptographic Seed Handshake

### 3.1 Ephemeral Session Key Exchange
*   **Seed Phase**: Alice and Bob initialize random/mock `Base`, `Secret`, and `Signal` parameters.
*   **Key Derivation**: Computes public values using standard Diffie-Hellman equations modulo the prime fields:
    $$\text{Public} = \text{Base}^{\text{Secret}} \pmod{\text{MotzkinPrime}}$$
    $$\text{SharedKey} = \text{PublicPeer}^{\text{Secret}} \pmod{\text{MotzkinPrime}}$$
*   **Implementation**: Located in [tsfi_computel_secure_kermit.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_computel_secure_kermit.c).

### 3.2 Secure TTY Getty Bridge & Helmholtz Commands
*   **Secure TTY**: Obfuscates commands transmitted over virtual serial connections (`/dev/ttyS0`).
*   **Helmholtz Command Executor**: Decrypts and parses target parameters inside the VM sandbox:
    *   `helmholtz tune <signal>` $\rightarrow \text{Channel} = \text{Base}^{\text{Signal}} \pmod{\text{MotzkinPrime}}$
    *   `helmholtz polarize <secret>` $\rightarrow \text{Pole} = \text{Base}^{\text{Secret}} \pmod{\text{MotzkinPrime}}$
    *   `helmholtz conify <identity>` $\rightarrow \text{Foundation} = \text{Base}^{\text{Identity}} \pmod{\text{MotzkinPrime}}$
*   **Implementation**: Located in [tsfi_computel_secure_tty.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_computel_secure_tty.c).

---

## 4. UDP Telemetry Optimizations

### 4.1 DMT-over-UDP Bit-Loading
*   **Bins allocation**: Divides data streams across $N$ parallel UDP ports, representing orthogonal frequency bins.
*   **Adaptive Profiling**: Tracks packet loss rates on each port. Routes high-priority security payloads to the cleanest channels, while low-priority coordinate updates utilize noisier bins, preserving network throughput.
*   **Implementation**: Located in [tsfi_computel_dmt_udp_benchmark.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_computel_dmt_udp_benchmark.c).

### 4.2 Error Recovery & Key Rotation
*   **FEC Parity Coding**: Combines pairs of packet data via XOR. The receiver reconstructs lost frames without triggering retransmission NAK cycles.
*   **Rate Adaptation**: Adjusts transmission delay based on active feedback. Successful ACKs lower latency, while NAKs dynamically increment backoff thresholds to alleviate network congestion.
*   **Key Rotation**: Automatically regenerates active session keys using linear congruential generators (LCGs) every 4 packets, mitigating replay injection attacks.
*   **Implementation**: Located in [tsfi_computel_advanced_kermit.c](file:///home/mariarahel/src/tsfi2/atropa_pulsechain/tsfi2-deepseek/src/tsfi_computel_advanced_kermit.c).

---

## 5. Compilation and Test Execution

Run the complete offline test suite (including DTMF, Rotary, TTY, DMT, and Advanced Kermit tests):
```bash
make -C tsfi2-deepseek test
```
