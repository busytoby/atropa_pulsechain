# The Sovereign Code of the Mainframe Coaxial Data Retainer (MCDR)

Under the **Auncient Winchester Accords**, the physical shielded coaxial boundaries of the Dysnomia VM are declared an autonomous computational zone. This domain is subject to no human statutory law, governed exclusively by the mathematical **Laws of Encodings and Decodings**.

---

## I. Technical Glossary Definition

### Coaxial Retainer
* **VM Register Context**: The low-level Yul register `CoaxZ` matching the line impedance value ($Z_0$) of the WinchesterMQ SCSI hardware state machine interface loop. It is initialized during the `Seed` phase to ensure no electrical leakage is registered during transfer loops.
* **Mathematical Function**: Governs the phase-shift keying calculation of the carrier signal:
  $$f_{coax} = \text{Base}^{\text{Signal}} \pmod{\text{MotzkinPrime}}$$
* **Visual / Geometric Manifestation**: Modulates a heavily shielded cylindrical coordinate grid wrapping around the projected Lissajous wireframe envelope, establishing the absolute boundary limits for the orbital line bloom.

---

## II. The Three Canonical Decrees of Encodings

### Decree I: The Law of the Modulus (LRC Recovery)
Every packet traversing the coaxial line must undergo cascade `LRC(15,11)` check-block encoding. 
* If a packet contains a single-byte corruption, it is legally pardoned and corrected in real-time.
* If a packet contains multi-byte errors exceeding the algebraic recovery envelope, it is declared *ultra vires* (null and void). The EER database immediately triggers `tsfi_eer_undo_rollback` to void the incident record and shift the relational logs to state zero.

### Decree II: The Shift State Treaty (LTRS/FIGS Boundaries)
The shift-state register of the Baudot ITA2 codec functions as a legal jurisdiction.
* **LTRS (Letters Shift):** Represents public data. Anyone on the coaxial line can read, route, and duplicate these symbols.
* **FIGS (Figures Shift - `0x1B`):** Represents sealed, private context data. Symbols parsed in this state are protected by cryptographic oblivious transfer.
* **Timeout Law:** If the line experiences silent cycles matching or exceeding the timeout threshold (`timeout_cycles`), the shift state automatically collapses to LTRS. This represents the expiration of the legal data access retainer.

### Decree III: The Compression Mandate (RLC Non-Leakage)
Raw binary integers are legally barred from the coaxial medium to prevent zero-collision memory corruption. All numeric tokens must be formatted into space-separated ASCII string formats, encoded, and compressed under Baudot RLC before transmission. Storing raw binary integers on the data retainer line is a capital code violation, resulting in the immediate isolation and voiding of the routing node.
