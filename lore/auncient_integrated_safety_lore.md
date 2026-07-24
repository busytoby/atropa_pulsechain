# The Trinity of Integrated Safety in the Auncient VM

## 1. The Three Principles of the Shield
To protect guest VDM partitions running under z/VM from memory corruption, timing side-channels, and unauthorized access, the TWS compiler and PL/EXUS ALU combine three complementary safety paradigms:

```
                  Unified Auncient Safety Shield
                                │
       ┌────────────────────────┼────────────────────────┐
       ▼                        ▼                        ▼
 [ Principle 1: Static ]  [ Principle 2: Dynamic ]  [ Principle 3: Converse ]
  Hoare Proof Assist       PDL Implication Gate      Converse Rollback (alpha^-)
  - Zero runtime cost      - Gated by PLL carrier    - Pops shadow stack logs
  - Audits privilege tags  - Audits frequencies      - Restores Merkle roots
```

### Principle 1: Static Hoare Proof Verification
Before any binary is compiled, the TWS proof assistant evaluates Hoare triples ($\{\phi\} \pi \{\psi\}$) representing static privilege and boundary rules. By executing symbolic dry-runs at compile time, the assistant verifies that the program $\pi$ preserves the invariants. If it passes, the checks are omitted from the runtime binary, ensuring **zero instruction overhead**.

### Principle 2: Dynamic PDL Implication Gating
Dynamic, signal-dependent parameters (such as coaxial carrier frequencies and resonance quality factors) cannot be evaluated at compile time. Instead, they are routed through PDL implication gates ($?\phi_{\text{fourier}} ; ?\psi_{\text{clearance}}$) at runtime, synchronized to the PLL clock lock points.

### Principle 3: Converse Rollback Recovery
If a transaction violates a postcondition at runtime, the supervisor executes the converse program ($\alpha^-$). The converse program pops shadow logs off the recovery stack and restores the VDM partition's register values and Merkle root hashes atomically, preventing state pollution.

---

## 2. Glossary Modulation Specifications

### StaticAssert
* **VM Register Context**: Register in the TWS compiler frontend indicating whether a static Hoare contract proof has succeeded, allowing compiler output emission.
* **Mathematical Function**: Proof validation mapping:
  $$P_{static} = \forall s . ( \phi(s) \implies \psi(\text{Execute}(\pi, s)) )$$
* **Visual / Geometric Modulation**: Modulates the frequency multiplier ($f_z$) of the Z-axis coordinate, altering the visual depth layers of the wireframe.

### DynamicGate
* **VM Register Context**: Register in the PL/EXUS ALU controlling the NPN Black conduction gate, asserted high only when dynamic carrier checks pass.
* **Mathematical Function**: Computes carrier signal verification:
  $$G_{dynamic} = \text{FreqMatch} \land \text{QFactorMatch}$$
* **Visual / Geometric Modulation**: Governs the camera's orbital velocity scaling and line bloom radius in the projected Lissajous grid.

### ConverseRestore
* **VM Register Context**: Stack pointer register tracking the converse shadow log recovery address.
* **Mathematical Function**: Pops the before-image status from the recovery stack:
  $$S_{restore} = \text{Pop}(S_{recovery})$$
* **Visual / Geometric Modulation**: Alters the starting phase angle shift ($\phi_w$) of the coordinate projection, rotating the wireframe backward to align with prior configurations.
