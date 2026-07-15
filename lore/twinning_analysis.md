# Twinning Calibration & Tonewheel Geometry Analysis

This document records and analyzes the physical tonewheel configurations, emotional keycode modulos, and geometric projections utilized in the Consensus Twinning Dashboard.

---

## 1. Physical Tonewheel baselines (Exponents)

Each of the five LAU bear circuits acts as an independent tonewheel with a fixed exponent representing its unique physical base speed or gear capability:

| Bear Identifier | Address Mapping (Truncated) | Exponent (Base Speed) | Panning Position |
| :--- | :--- | :--- | :--- |
| **Judge Bear** | `0xAD4e198623A5E2723e19E4...` | `440` | `0.0` (Center) |
| **Witness A** | `0xD07B9f3DF4E9634EbAa0...` | `1200` | `-0.6` (Left-Mid) |
| **Witness B** | `0xd32c39fee49391c7952d...` | `880` | `+0.6` (Right-Mid) |
| **Juror 1** | `0xed343c0f99c89ed7c3c9...` | `443` | `-0.8` (Far Left) |
| **Juror 2** | `0x3e10ed242ecb3951151e...` | `552` | `+0.8` (Far Right) |

---

## 2. Emotional Modulo Calibrations (Understanding Targets)

Rather than storing complex multi-chord lookup databases, emotional communication is mapped directly to keycode modulos matching low-level hardware register values:

| Target Emotion | Modulo Target ($Pi \pmod{\text{exp}}$) | Keycode Representation | Harmonic Quality |
| :--- | :--- | :--- | :--- |
| **JOY** | `32` | `'d' / 'D'` | Consonant 1:1 gear alignment |
| **SORROW** | `30` | `'a' / 'A'` | Minor 3:2 gear offset |
| **ANGER** | `13` | `Return` | Spiky sawtooth distortion |
| **FEAR** | `27` | `Escape` | Shivering frequency jitter |
| **MELANCHOLY** | `64` | `'@'` | Concentric slow-decay rose orbits |
| **EUPHORIA** | `77` | `'m' / 'M'` | High-velocity wide double-loops |
| **SUSPENSE** | `83` | `'s' / 'S'` | Off-center eccentric orbit wobble |
| **TRANQUILITY** | `84` | `'t' / 'T'` | Low-frequency baseline harmony |
| **CONFUSION** | `63` | `'?'` | High-entropy asymmetrical drift |

---

## 3. Visual & Geometric Mappings (Hypotrochoid Projections)

The dynamic coordinate projection of each bear's state $Pi$ maps to the visual oscilloscope spindle using standard hypotrochoid equations modulated by the selected emotion's physical characteristics:

### JOY / TRANQUILITY
*   **Geometric Manifestation**: Renders symmetric, smooth, well-connected clover or rosette curves.
*   **Aesthetic Quality**: High line connection density (small step size) with a soft outer glow. Represents perfect resonance.

### ANGER
*   **Geometric Manifestation**: Sharp, jagged star-like spikes radiating outward from the center.
*   **Aesthetic Quality**: Achieved by applying a high-frequency sawtooth overlay to the radial distance, creating sharp geometric vertices.

### SORROW
*   **Geometric Manifestation**: Asymmetrical teardrop profiles sagging along the negative Y-axis.
*   **Aesthetic Quality**: Modulates coordinate offsets to squeeze the upper envelope while pulling coordinates downward.

### FEAR
*   **Geometric Manifestation**: Constantly shivering, tightly wound, unstable coordinates.
*   **Aesthetic Quality**: Injects a high-frequency spatial jitter offset even under locked consensus, representing mechanical instability.

### MELANCHOLY
*   **Geometric Manifestation**: Densely packed concentric orbits rotating at a very slow velocity.
*   **Aesthetic Quality**: Modulates a slow sinusoidal contraction envelope over the radius ($R_{hyp}$).
