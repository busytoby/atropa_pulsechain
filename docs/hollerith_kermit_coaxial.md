# Hollerith Kermit Protocols over Coaxial Lines

This document details the configuration, routing mechanisms, and signal pathways for running Hollerith Kermit File Transfer Protocols over physical coaxial lines. The architecture supports two operational modes: **Tone Wheel Modulated Mode** (Acoustic Link) and **Baseband Digital Mode** (Direct Digital Link).

---

## 1. Transmission Path Topology

```
                  ┌────────────────────────────────────────────────────────┐
                  │                 K E R M I T   P A C K E T              │
                  │  [SOH] [LEN] [SEQ] [TYPE] [DATA/CHARACTERS] [CHECKSUM]  │
                  └───────────────────────────┬────────────────────────────┘
                                              │
                    ┌─────────────────────────┴─────────────────────────┐
                    ▼ (Modulation Mode Selector)                        ▼
     ┌─────────────────────────────┐                     ┌─────────────────────────────┐
     │ Tone Wheel Modulated        │                     │ Baseband Digital            │
     ├─────────────────────────────┤                     ├─────────────────────────────┤
     │ * 8-Bit EBCDIC/ASCII select │                     │ * Packets serialized to     │
     │ * Acoustic FSK Carrier      │                     │   Manchester/NRZ bitstream  │
     │ * Verified via SAGE PLL     │                     │ * Driven directly on Coax   │
     └──────────────┬──────────────┘                     └──────────────┬──────────────┘
                    │                                                   │
                    └─────────────────────────┬─────────────────────────┘
                                              ▼
                                   ┌─────────────────────┐
                                   │  COAXIAL BACKPLANE  │
                                   └─────────────────────┘
```

---

## 2. Tone Wheel Modulated Mode (Acoustic Coaxial Link)
In this configuration, the system routes Kermit characters through the physical/optical rotating **Hollerith Tone Wheels** to provide maximum noise rejection over long coaxial runs:

1. **Packet Framing:** Kermit envelopes (`SOH`, `LEN`, `SEQ`, `TYPE`, `DATA`, `CHECK`) are serialized into character bytes (ASCII, EBCDIC, or Baudot/ITA2).
2. **Harmonic Gating:** Characters select coordinate tracks on the rotating cylinder. The electromechanical contact brushes or photodiode detectors translate the punches into continuous-wave (CW) audio tones or Frequency-Shift Keying (FSK) carrier signals.
3. **PLL Synchronization:** The receiving node maps the acoustic signal from the coax line into its **SAGE PLL Fabric** (`tsfi_sage_pll_fabric`). The phase comparator correlates the incoming phase profile with reference tracks on the receiver's local wheel, decoding the packet with zero digital processing overhead.

---

## 3. Baseband Digital Mode (Direct Digital Coaxial Link)
When high-bandwidth data transfers are preferred and line noise is low, the tone wheel mechanism is bypassed:

1. **Serializer Bypass:** The Kermit frames are serialized directly into a digital bitstream (using Manchester encoding, NRZ, or raw TTL level shifts).
2. **Coaxial Driving:** The digital line drivers push the bitstream directly onto the coaxial cable as discrete voltage step transitions.
3. **Queue Ingestion:** The receiving coaxial transceivers decode the digital bitstream and push the packets directly to the CICS queue buffer managers, completing the transfer without mechanical latency or analog PLL lock stages.
