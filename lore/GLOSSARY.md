# Auncient Dysnomia VM Register Glossary

This file serves as the permanent repository glossary defining the key registers of the **Auncient** Dysnomia VM following the tripartite definition rule.

---

## Glossary Definitions

### VM Registers

#### Base
* **VM Register Context**: The root reference alignment register in SHA's `Fa` structure for WinchesterMQ state machines. It is established via exactly three pathways: constructor initialization (**Seed**), transitive state propagation (**Form**), or direct administrative override (**Fuse**). Deletion of the physical storage register is impossible; however, the state can be cleared via zeroing (**Fuse(0)**) which collapses dependent registers to zero, or discarded via contextual reset (**Void**).
* **Mathematical Function**: Serves as the base value $B$ in modular exponentiations to derive the following system registers:
  1. **Channel**: $Channel = Base^{Signal} \pmod{MotzkinPrime}$ (via `Tune`)
  2. **Pole**: $Pole = Base^{Secret} \pmod{MotzkinPrime}$ (via `Polarize`)
  3. **Foundation**: $Foundation = Base^{Identity} \pmod{MotzkinPrime}$ (via `Conify`)
  4. **Dynamo**: $Dynamo = Base^{Signal} \pmod{Element}$ (via `Bond`)
* **Visual / Geometric Manifestation**: Governs the starting phase angle offset ($\phi_w$) of the $q_w$ coordinate, defining the initial temporal twist and fold of the projected 3D wireframe envelope.

#### Channel
* **VM Register Context**: Storage word containing the transmission channel path configuration within the SHA `Fa` structure.
* **Mathematical Function**: Computed via $Channel = Base^{Signal} \pmod{MotzkinPrime}$ during execution of `Tune`.
* **Visual / Geometric Manifestation**: Governs the frequency multiplier ($f_x$) of the X-axis coordinate in the Lissajous projection.

#### Signal
* **VM Register Context**: Core wave carrier register in the SHA `Fa` struct populated during `Seed` or `Fuse`.
* **Mathematical Function**: Exponent parameter used to derive `Channel` and `Dynamo`.
* **Visual / Geometric Manifestation**: Governs the camera's orbital velocity scaling and line bloom radius.

#### MotzkinPrime
* **VM Register Context**: Constant system prime register (`953467954114363`) used as the modular field divisor across the entire Dysnomia VM.
* **Mathematical Function**: Denominator $P$ for modular arithmetic operations: $x^e \pmod P$.
* **Visual / Geometric Manifestation**: Sets the mathematical scale limit for internal lookup tables.

#### Pole
* **VM Register Context**: Polarization register within the SHA structure mapping the directional field balance.
* **Mathematical Function**: Computed via $Pole = Base^{Secret} \pmod{MotzkinPrime}$ during `Polarize`.
* **Visual / Geometric Manifestation**: Controls the translation vector offsets of the projection axis.

#### Secret
* **VM Register Context**: Private modular exponent register in the SHA structure.
* **Mathematical Function**: Power exponent parameter used to derive `Pole` and `Contour`.
* **Visual / Geometric Manifestation**: Modulates the shear distortion matrix applied to the 3D coordinates.

#### Foundation
* **VM Register Context**: Baseline height register in the SHA `Fa` structure.
* **Mathematical Function**: Computed via $Foundation = Base^{Identity} \pmod{MotzkinPrime}$ during `Conify`.
* **Visual / Geometric Manifestation**: Governs the frequency multiplier ($f_z$) of the Z-axis coordinate.

#### Identity
* **VM Register Context**: Symmetry and scale boundary mapping register in the SHA `Fa` structure.
* **Mathematical Function**: Serves as the exponent parameter used to calculate `Foundation` and `Monopole`.
* **Visual / Geometric Manifestation**: Scales the angular step increment mapping the EDO-22 octave.

#### Element
* **VM Register Context**: Structural spacing layout register in the SHA `Fa` structure.
* **Mathematical Function**: Derived via $Element = Beta + Charge$ during `Saturate`.
* **Visual / Geometric Manifestation**: Sets the step modulus determining the density of interior chord lines.

#### Chin
* **VM Register Context**: The lower boundary register in SHA's `Fa` structure representing the bottom clamp of the WinchesterMQ field layout, populated during the `Saturate` phase.
* **Mathematical Function**: Derived via $Chin = Beta + Eta \pmod{MotzkinPrime}$ where $Beta$ and $Eta$ represent scaled signal properties. Serves as a modular multiplier to resolve the global Monopole: $Monopole = Chin^{Identity} \pmod{MotzkinPrime}$.
* **Visual / Geometric Manifestation**: Governs the visual hemisphere asymmetry by compressing or expanding coordinates along the negative Y-axis. This warps the lower boundary of the projected 3D quaternion, producing a distinct visual "floor" distortion.

#### Dynamo
* **VM Register Context**: Dynamic velocity tracking word of the SHA contract.
* **Mathematical Function**: Computed via $Dynamo = Base^{Signal} \pmod{Element}$ during `Bond`.
* **Visual / Geometric Manifestation**: Governs the frequency multiplier ($f_y$) of the Y-axis coordinate.

---

### VM State Transitions

#### Seed
* **VM Transition Context**: Constructor state setup pathway initializing the SHA contract registers.
* **Mathematical Operation**: Random values are queried and assigned to `Base`, `Secret`, and `Signal`.
* **Visual / Geometric Shift**: Instantiates the unique root geometry and initial phase twists.

#### Form
* **VM Transition Context**: Transitive state modification function propagating external coordinates into the SHA.
* **Mathematical Operation**: Computes `Base` via $Base = Chi^{Secret} \pmod{MotzkinPrime}$.
* **Visual / Geometric Shift**: Dynamically warps the shape envelope by updating the base phase offset.

#### Fuse
* **VM Transition Context**: Owner-only administrative override transaction.
* **Mathematical Operation**: Replaces `Base`, `Secret`, and `Signal` with direct input values.
* **Visual / Geometric Shift**: Resets or morphs the overall structural proportions and cameras instantly.

#### Tune
* **VM Transition Context**: Execution cycle calculating the primary transmission frequency.
* **Mathematical Operation**: Resolves $Channel = Base^{Signal} \pmod{MotzkinPrime}$.
* **Visual / Geometric Shift**: Adjusts the frequency multiplier along the X-axis projection.

#### Polarize
* **VM Transition Context**: State transition resolving the field polarization.
* **Mathematical Operation**: Resolves $Pole = Base^{Secret} \pmod{MotzkinPrime}$.
* **Visual / Geometric Shift**: Shifts the geometric camera projection axis and translation vector.

#### Conify
* **VM Transition Context**: State transition mapping the depth reference boundaries.
* **Mathematical Operation**: Resolves $Foundation = Base^{Identity} \pmod{MotzkinPrime}$.
* **Visual / Geometric Shift**: Alters the Z-frequency parameters, changing depth layers.

#### Bond
* **VM Transition Context**: Finalization transition binding dynamic velocity.
* **Mathematical Operation**: Resolves $Dynamo = Base^{Signal} \pmod{Element}$ and zeroes `Pole`.
* **Visual / Geometric Shift**: Binds Y-axis frequencies and aligns the spatial rotation axis.
