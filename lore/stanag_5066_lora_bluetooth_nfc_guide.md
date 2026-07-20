# Adapting NATO STANAG 5066 over Alternative Transports

This document outlines the architectural specifications and protocol adaptations required to transmit NATO STANAG 5066 link-layer frames over LoRa, Bluetooth, and NFC. These specifications ensure compatibility with the standard verifiers implemented within the Auncient virtual hardware registry.

---

## 1. LoRa Transport Configuration

LoRa provides long-range, low-power broadcast capabilities but introduces strict packet size limits and high times-on-air.

### Frame Parameters
* **Segment Size Constraint**: Clamped to **128 bytes** (standard default is up to 2048 bytes) to fit within the physical LoRa packet size constraints.
* **Sync Sequence**: The 16-bit sync sequence (`0x90EB` / `0xEB90`) is prepended to the software payload, allowing the software decoder to quickly identify NATO frames in the received demodulated bitstream.
* **Sliding Window (ARQ)**: Clamped to a window size of **4 to 8 frames** to prevent channel congestion and timeout failures on high Spreading Factor (SF) links.

---

## 2. Bluetooth Transport Configuration

Bluetooth provides high-reliability, short-range connections with native link-layer error correction.

### Frame Parameters
* **Transmission Mode**: Set to **Non-ARQ (Broadcast)**. Because Bluetooth L2CAP/RFCOMM natively guarantees delivery, STANAG-level sliding window acknowledgments are redundant.
* **Short-TIN Mapping**: The 26-bit NATO node address is mapped directly onto the lower 3 bytes of the Bluetooth device MAC address.
* **Sync Sequence**: Handled at the software framing layer inside the L2CAP packet channel payload.

---

## 3. Near Field Communication (NFC) Adaptation

NFC is a transactional, half-duplex, close-proximity transport layer.

### Frame Parameters
* **Transmission Window**: Set to **1** (Stop-and-Wait). Multi-frame sliding windows are disabled due to the short-range transactional nature of the link.
* **MTU Boundary**: Clamped to **64 or 128 bytes** to fit within small NDEF record buffers.
* **Address Resolution**: Short-TIN addresses map directly to designated NFC target identifier tags.

---

## 4. Security & Invariance Constraints

* **TIN Isolation**: All 9-digit IRS Taxpayer Identification Numbers (TINs) must remain isolated from the 26-bit STANAG Short-TIN node addresses. Any IRS TIN numeric value must not overlap into the node address range.
* **FCS Independence**: Frame Check Sequence (FCS) verification flags (CRC-16 or CRC-32) are validated independently and must never bypass the namespace isolation checks.
