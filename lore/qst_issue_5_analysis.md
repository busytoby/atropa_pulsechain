# 📰 QST Magazine Issue #5 (April 1916) & TSFi2 Decentralized Relay Coordination

This document details the historical and technical findings of **QST Issue #5** (April 1916, Vol. 1, No. 5), focusing on Pennsylvania State College's station **8XE** and the introduction of the **Type "T" Tubular Audion**, showing their connection to the decentralized coordination and physical valve models of the **TSFi2** VM.

---

## 1. Key Articles & Architectural Alignment

| QST Issue #5 Article / Concept | Historical Technical Detail | TSFi2 Implementation Translation |
| :--- | :--- | :--- |
| **"8XE"** (M. R. Strausberger) | Penn State College's experimental station with a 121-foot insulated tower, 2 kW Clapp-Eastham type "E" transformer, rotary gap transmitter, variometer tuner, Stromberg-Carlson headphones. | Emulation of transformer core saturation, rotary gap spark frequency, and variometer impedance matching inside `tsfi_resonance.c`. |
| **"Washington's Birthday Amateur Relay Message"** (W. H. Kirwan) | Nationwide relay demonstrating amateur efficiency. Message originated at Rock Island Arsenal (9XE) and passed through trunk lines to public officials. | Validation of routing algorithms and peer-to-peer relaying structures modeled in `tsfi_comm.c` and dynamic multi-agent messaging. |
| **Type "T" Tubular Audion Announcement** | Advertisement introducing the new tubular geometry bulb featuring a cylindrical plate (anode) and a helical coil grid surrounding a straight axial filament. | Implementation of cylindrical coordinate electrostatic fields ($E(r) \propto 1 / r$) and reduced inter-electrode capacitance in the valve engine. |

---

## 2. Deep-Dive: Type "T" Tubular Audion Cylindrical Geometry

Prior to the Type "T" Tubular Audion, de Forest Audion bulbs used flat plates and grids positioned adjacent to a looped filament (flat plane geometry). The tubular design introduced in April 1916 rearranged these elements coaxially:

### A. Flat Plane vs. Cylindrical Electrostatic Fields
* **Flat Plane Audion (Standard):** The electrostatic field is uniform across the parallel planes:
  $$E_{\text{flat}} = \frac{V_p}{\text{distance}}$$
* **Tubular Audion (Cylindrical):** The electric field between the central axial filament ($r_a$) and the outer cylindrical plate ($r_b$) is non-uniform, concentrating the field intensity near the emitter surface:
  $$E_{\text{cylindrical}}(r) = \frac{V_p}{r \cdot \ln(r_b / r_a)}$$
  This non-uniform field shifts the space-charge density, slightly altering the knee of the plate current $I_p$ curve and lowering inter-electrode capacitance.

### B. Cylindrical Permittivity & Capacity
The capacitance of the tubular Audion is modeled using cylindrical boundary equations:
$$C_{\text{tubular}} = \frac{2 \pi \epsilon_0 \epsilon_r L}{\ln(r_b / r_a)}$$
This reduces stray inter-electrode capacitance, making the Audion significantly more stable at higher frequencies.

---

## 3. Washington's Birthday Relay Routing Simulation

Kirwan's relay of Colonel Nicholson's message was a milestone in proving that a decentralized group of operators could coordinate under a uniform protocol.
* **Protocol Invariant:** The message propagation relied on hand-shaking and acknowledgment (similar to TCP or the on-chain Merkle confirmation blocks).
* **TSFi2 Relay Test:** The communication system (`tsfi_comm.c`) includes benchmark tests verifying that a packet can be routed through multiple simulated hop nodes using optimized routing protocols without losing data integrity ($\Phi = 0$).
