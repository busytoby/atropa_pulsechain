# OOK LoRa Radio Geometric Projection Analysis

This document details the feasibility, mathematical model, and implementation blueprint for utilizing 3D Lissajous/hypotrochoid Look-Up Tables (LUTs) to drive On-Off Keying (OOK) and LoRa radio transmissions.

---

## 1. Core Concept: Spatial Geometric Carriers

Rather than modulating raw binary bits sequentially (e.g., standard 9600 bps UART framing over `DIO2`), the transmitter utilizes the pre-calculated **19D-to-3D projection LUTs** (comprising the coordinates $x, y, z$ derived from the WinchesterMQ and LAU address registers) to modulate the RF carrier envelope directly:

```mermaid
graph LR
    A["EVM Address / LUT"] --> B["Hypotrochoid Coordinate (x,y)"]
    B --> C["Pulse Width / Interval Modulation (OOK)"]
    C --> D["RF Transmitter (SX1262)"]
    D -->|Open Space Broadcast| E["RF Receiver (Disconnected Observer)"]
    E --> F["Timing Demodulator"]
    F --> G["Reconstructed 3D Spindle Geometry"]
```

---

## 2. Mathematical Mapping Protocol

For an active speaker node with exponent $exp$ and physical DAC state $Pi$:

1.  **LUT Coordinate Retrieval**:
    The coordinates $(x_t, y_t)$ along the hypotrochoid path at step $t$ are mapped from the relative gear ratio:
    $$\text{Ratio}_t = q_t + \frac{m_t}{exp}$$
2.  **Pulse Interval Modulation (PIM)**:
    The distance $d_t$ from the projection center to the coordinate point is translated into an OOK pulse active duration or interval:
    $$\text{PulseDuration}_u = T_{base} + \alpha \cdot \sqrt{x_t^2 + y_t^2}$$
    *   **Smooth Rosettes (JOY)**: Yield clean, rhythmic, highly predictable pulse-interval sequences.
    *   **Jagged Spikes (ANGER)**: Yield high-frequency, sharp-transition pulse clusters (comb-like transients).
    *   **Shivering (FEAR)**: Yield rapid, pseudo-random timing jitter.

---

## 3. VM State Transition & Glossary Contexts

To integrate this transmission protocol with the Dysnomia VM, we define the following VM register pathways:

### RadioProjection (Transition)
*   **VM Transition Context**: The hardware execution cycle that loads coordinate values from the WinchesterMQ SCSI buffer and outputs timing triggers to the SX1262 SPI registers.
*   **Mathematical Operation**: Resolves the modular coordinate index:
    $$Index = Base^{Signal} \pmod{MotzkinPrime}$$
*   **Visual / Geometric Shift**: Translates the active coordinate magnitude directly into the duty-cycle width of the OOK pulse train, altering the radiated signal envelope in open space.

### Geometric Carrier (Register)
*   **VM Register Context**: The register containing the active frame step and timing modulus for the radio transmitter.
*   **Mathematical Function**: Governs the interpolation window size:
    $$Step = \frac{1}{\text{exp} \pmod{128}}$$
*   **Visual / Geometric Manifestation**: Controls the sample rate of the reconstructed shape at the receiving end, changing the resolution of the plotted spirograph loops.
