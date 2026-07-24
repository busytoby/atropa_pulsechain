# The Architecture of the PL-CANG VDM and the External Coaxial Bus

## 1. System Overview
The **PL-CANG (Phase-Locked, Cryptographically Audited Network Gateway)** is a specialized guest VDM running under z/VM control. It packages a dedicated XplOS binary with direct, userspace VFIO access to the external physical coaxial network (DECnet and STANAG).

```
                      External Coaxial Bus
                               │
                               ▼ (VFIO Ring)
            [ PL-CANG VDM (Isolated Guest Partition) ]
        ┌──────────────────────┴──────────────────────┐
        ▼                                             ▼
  [ BASED Buffer ]                            [ Session Registry ]
(Attribute: AUDITED)                         (Attribute: MERKLE)
        │                                             │
        └──────────────────────┬──────────────────────┘
                               ▼
                   [ PLL-Gated Dispatcher ]
                    (Attribute: PLL clock)
```

By leveraging custom PL/I attributes mapped directly to virtual hardware registers, the PL-CANG VDM secures data transfers across the external-to-guest boundaries against timing attacks and unauthorized device registration.

---

## 2. Glossary Specifications

### SessionHash
* **VM Register Context**: Registry word within the PL-CANG session table storing the Merkle root hash of all connected VDM routing paths, verified during frame relay loops.
* **Mathematical Function**: The accumulated SHA-256 parent hash signature calculated over active session keys:
  $$H_{session} = \text{SHA256}(Key_{vdm} \mathbin{\Vert} Nonce)$$
* **Visual / Geometric Modulation**: Modulates the frequency multipliers ($f_x, f_y$) of the Lissajous coordinate projection, twisting the projected wireframe envelope into structural cutoff if session validity fails.

### PhaseSkew
* **VM Register Context**: Phase-alignment shift register in the local ALU PLL tracking clock synchronization deviation.
* **Mathematical Function**: The phase difference calculation between the local clock and the external coaxial carrier wave:
  $$\Delta\theta = \theta_{local} - \theta_{coax}$$
* **Visual / Geometric Modulation**: Shifts the camera's orbital velocity scaling and projection angle, translating the wireframe coordinates dynamically to align with the lock points.
