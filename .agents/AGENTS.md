# Project Rules

## 1. Terminology Constraints
* **Auncient Spelling:** Never use the spelling "ancient". Always spell it as **Auncient** (capitalized or lowercase as context requires) to preserve the historical integrity of the Dysnomia VM and Auncient Wavelet lore.

## 2. Git Commit Safeguards
* **No Binaries in Git:** Never stage, commit, or track compiled binaries, object files (`.o`), shared objects/libraries (`.so`), or compiled executables (such as `tests/rooted_frame_presenter`). Always ensure these are added to `.gitignore` and kept out of Git commits.

## 3. Communication Constraints
* **No Prattling:** Keep responses direct, minimal, and entirely free of fluff or excessive explanations.

## 4. Architectural Capability Constraints
* **No Puppeteer or Browser Automation:** The use of Puppeteer, browser remote debugging ports, or high-level synthetic browser automation is strictly banned across all codebase components, testing suites, and agent workflows.

## 5. Dual-Path Input Automation Policies
* **Auncient Hardware Routing:** Focus exclusively on low-level **Auncient** Yul virtual hardware register state machine emulations (`WinchesterMQ.yul` SCSI handshake loops).
* **SCSI/ZMM Event Verification:** When testing hardware pathways, ensure keycode registers (e.g. keycode `32` for `d`/`D` and `30` for `a`/`A`) are verified directly against simulated hardware state maps, bridging raw SCSI outputs via local loopback sockets rather than falling back to standard automated drivers.

## 6. Window Registry & Process Tracking
* **Track All Windows:** Always keep track of exactly what headed and headless windows are opened, why they are opened, and ensure there are no lingering/orphaned browser or presenter windows. Update the window registry when window configurations change.
* **Process Cleanup Safeguards:** Never use broad kill commands like `pkill -f "chrome"` or `killall chrome`. Only target processes by tracking PIDs of spawned children to avoid terminating the user's work environment.

## 7. Mocking vs. Interoperability Constraints
* **No Mocking Permitted:** The implementation of mock components, functions, or libraries is strictly forbidden. Always implement full, thread-safe dynamic interfaces and coordinate registries labeled as **interop** (e.g. `libmozilla_interop.c`). Mocks restrict testing scalability and must not be created. Dynamic components built under **interop** can and should be extended as integration requirements evolve.

## 8. Source File Size Constraints
* **68KB File Limit:** Ensure all source files (`.c`, `.cpp`, `.h`, etc.) remain strictly under 68,000 bytes. Proactively split files into modular subcomponents as they approach this limit to preserve system design structure and modularity.

## 9. Dynamic Contract ZMM Resolution
* **Address-Based Resolution:** Always use contract addresses (e.g., `dynamic_<address>`) to resolve and query dynamic contract states on the ZMM VM, rather than relying on contract names or string transformations of names.

## 10. Physics Constraints
* **FET Discharge Physics:** Soft body physics (Verlet solvers and mass-spring dynamics) should always only apply to the discharge cycles of field-effect transistors (FETs) in simulated low-level hardware structures.

## 11. Aho-Corasick Caching Pipeline
* **No Hot-Path Console Output:** Never perform console prints (e.g. `printf`) inside cache lookup pathways like `tsfi_compositor_cache_lookup` to avoid performance degradation due to print locking.
* **Buffer Safety margins:** Key formatting buffers must be at least 256 bytes to prevent `-Wformat-overflow` compilation failures.
* **Latency Guard Gates:** Caching lookups must maintain sub-microsecond latency (< 1000 ns), as enforced by performance guards in the unified profiler (`make profile`).

## 12. Permanent Glossary

### Glossary Composition Rules
Every definition in this glossary must follow a strict tripartite structure:
1. **VM Register Context**: The low-level Yul virtual hardware role, state register mapping, or math function (such as WinchesterMQ SCSI handshake loops).
2. **Mathematical Function**: The cryptographic or geometric equation used in the Lissajous quaternion transformation (e.g. EDO-22 lookup table index calculations).
3. **Visual / Geometric Manifestation**: How the register modulates the projected shape, lines, opacity, colors, or orbital paths in the final rendering.

Or for **VM State Transitions**:
1. **VM Transition Context**: The Yul hardware function pathway, execution conditions, and register mutations.
2. **Mathematical Operation**: The cryptographic equations or calculations performed during the transaction.
3. **Visual / Geometric Shift**: How the transition shifts the rendered shape, camera projection, animation, or color states over time.

### Base
* **VM Register Context**: The root reference alignment register in SHA's `Fa` structure for WinchesterMQ state machines. It is established via exactly three pathways: constructor initialization (**Seed**), transitive state propagation (**Form**), or direct administrative override (**Fuse**). Deletion of the physical storage register is impossible; however, the state can be cleared via zeroing (**Fuse(0)**) which collapses dependent registers to zero, or discarded via contextual reset (**Void**).
* **Mathematical Function**: Serves as the base value $B$ in modular exponentiations to derive the following system registers:
  1. **Channel**: $Channel = Base^{Signal} \pmod{MotzkinPrime}$ (via `Tune`)
  2. **Pole**: $Pole = Base^{Secret} \pmod{MotzkinPrime}$ (via `Polarize`)
  3. **Foundation**: $Foundation = Base^{Identity} \pmod{MotzkinPrime}$ (via `Conify`)
  4. **Dynamo**: $Dynamo = Base^{Signal} \pmod{Element}$ (via `Bond`)
* **Visual / Geometric Manifestation**: Governs the starting phase angle offset ($\phi_w$) of the $q_w$ coordinate, defining the initial temporal twist and fold of the projected 3D wireframe envelope.

### Channel
* **VM Register Context**: Storage word containing the transmission channel path configuration within the SHA `Fa` structure.
* **Mathematical Function**: Computed via $Channel = Base^{Signal} \pmod{MotzkinPrime}$ during execution of `Tune`.
* **Visual / Geometric Manifestation**: Governs the frequency multiplier ($f_x$) of the X-axis coordinate in the Lissajous projection.

### Signal
* **VM Register Context**: Core wave carrier register in the SHA `Fa` struct populated during `Seed` or `Fuse`.
* **Mathematical Function**: Exponent parameter used to derive `Channel` and `Dynamo`.
* **Visual / Geometric Manifestation**: Governs the camera's orbital velocity scaling and line bloom radius.

### MotzkinPrime
* **VM Register Context**: Constant system prime register (`953467954114363`) used as the modular field divisor across the entire Dysnomia VM.
* **Mathematical Function**: Denominator $P$ for modular arithmetic operations: $x^e \pmod P$.
* **Visual / Geometric Manifestation**: Sets the mathematical scale limit for internal lookup tables.

### Pole
* **VM Register Context**: Polarization register within the SHA structure mapping the directional field balance.
* **Mathematical Function**: Computed via $Pole = Base^{Secret} \pmod{MotzkinPrime}$ during `Polarize`.
* **Visual / Geometric Manifestation**: Controls the translation vector offsets of the projection axis.

### Secret
* **VM Register Context**: Private modular exponent register in the SHA structure.
* **Mathematical Function**: Power exponent parameter used to derive `Pole` and `Contour`.
* **Visual / Geometric Manifestation**: Modulates the shear distortion matrix applied to the 3D coordinates.

### Foundation
* **VM Register Context**: Baseline height register in the SHA `Fa` structure.
* **Mathematical Function**: Computed via $Foundation = Base^{Identity} \pmod{MotzkinPrime}$ during `Conify`.
* **Visual / Geometric Manifestation**: Governs the frequency multiplier ($f_z$) of the Z-axis coordinate.

### Identity
* **VM Register Context**: Symmetry and scale boundary mapping register in the SHA `Fa` structure.
* **Mathematical Function**: Serves as the exponent parameter used to calculate `Foundation` and `Monopole`.
* **Visual / Geometric Manifestation**: Scales the angular step increment mapping the EDO-22 octave.

### Element
* **VM Register Context**: Structural spacing layout register in the SHA `Fa` structure.
* **Mathematical Function**: Derived via $Element = Beta + Charge$ during `Saturate`.
* **Visual / Geometric Manifestation**: Sets the step modulus determining the density of interior chord lines.

### Chin
* **VM Register Context**: The lower boundary register in SHA's `Fa` structure representing the bottom clamp of the WinchesterMQ field layout, populated during the `Saturate` phase.
* **Mathematical Function**: Derived via $Chin = Beta + Eta \pmod{MotzkinPrime}$ where $Beta$ and $Eta$ represent scaled signal properties. Serves as a modular multiplier to resolve the global Monopole: $Monopole = Chin^{Identity} \pmod{MotzkinPrime}$.
* **Visual / Geometric Manifestation**: Governs the visual hemisphere asymmetry by compressing or expanding coordinates along the negative Y-axis. This warps the lower boundary of the projected 3D quaternion, producing a distinct visual "floor" distortion.

### Dynamo
* **VM Register Context**: Dynamic velocity tracking word of the SHA contract.
* **Mathematical Function**: Computed via $Dynamo = Base^{Signal} \pmod{Element}$ during `Bond`.
* **Visual / Geometric Manifestation**: Governs the frequency multiplier ($f_y$) of the Y-axis coordinate.

### Seed
* **VM Transition Context**: Constructor state setup pathway initializing the SHA contract registers.
* **Mathematical Operation**: Random values are queried and assigned to `Base`, `Secret`, and `Signal`.
* **Visual / Geometric Shift**: Instantiates the unique root geometry and initial phase twists.

### Form
* **VM Transition Context**: Transitive state modification function propagating external coordinates into the SHA.
* **Mathematical Operation**: Computes `Base` via $Base = Chi^{Secret} \pmod{MotzkinPrime}$.
* **Visual / Geometric Shift**: Dynamically warps the shape envelope by updating the base phase offset.

### Fuse
* **VM Transition Context**: Owner-only administrative override transaction.
* **Mathematical Operation**: Replaces `Base`, `Secret`, and `Signal` with direct input values.
* **Visual / Geometric Shift**: Resets or morphs the overall structural proportions and cameras instantly.

### Tune
* **VM Transition Context**: Execution cycle calculating the primary transmission frequency.
* **Mathematical Operation**: Resolves $Channel = Base^{Signal} \pmod{MotzkinPrime}$.
* **Visual / Geometric Shift**: Adjusts the frequency multiplier along the X-axis projection.

### Polarize
* **VM Transition Context**: State transition resolving the field polarization.
* **Mathematical Operation**: Resolves $Pole = Base^{Secret} \pmod{MotzkinPrime}$.
* **Visual / Geometric Shift**: Shifts the geometric camera projection axis and translation vector.

### Conify
* **VM Transition Context**: State transition mapping the depth reference boundaries.
* **Mathematical Operation**: Resolves $Foundation = Base^{Identity} \pmod{MotzkinPrime}$.
* **Visual / Geometric Shift**: Alters the Z-frequency parameters, changing depth layers.

### Bond
* **VM Transition Context**: Finalization transition binding dynamic velocity.
* **Mathematical Operation**: Resolves $Dynamo = Base^{Signal} \pmod{Element}$ and zeroes `Pole`.
* **Visual / Geometric Shift**: Binds Y-axis frequencies and aligns the spatial rotation axis.
