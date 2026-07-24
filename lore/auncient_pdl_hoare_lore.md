# The Verification of Hoare Contracts in Auncient Compilers

## 1. Safety Contracts and Compile-Time Proofs
In standard compiler designs, verifying that compiled assembly binaries do not violate critical memory boundaries or execution privileges requires extensive runtime checking. However, in the Auncient compiler toolchain (TWS), safety is verified at compile time using Hoare triples of the form:

$$\{\phi\} \pi \{\psi\}$$

Within Propositional Dynamic Logic (PDL), this triple is compiled and proven as the modal implication formula:

$$\phi \implies [\pi]\psi$$

```
              Auncient Compile-Time Proof Gate
                             │
                             ▼
                 [ Initial State (tag, value) ]
                             │
                             ▼
                   Precondition Audit ?phi
                             │
              ┌──────────────┴──────────────┐
              ▼ (phi is True)               ▼ (phi is False)
         Symbolic Run of program pi    Skip Verification
              │                        (Trivially Safe)
              ▼
       Postcondition psi?
              │
       ┌──────┴──────┐
       ▼ (True)      ▼ (False)
    Approve Binary   Halt Compiler (Cutoff)
```

During compilation, the proof assistant performs a symbolic dry-run of program $\pi$ starting from any state satisfying precondition $\phi$. If the resulting state violates postcondition $\psi$, the assistant triggers a compilation trap, shifting the write gate to `TRANSISTOR_CUTOFF` and preventing the binary from emitting.

---

## 2. Glossary Modulation Specifications

### HoarePre
* **VM Register Context**: Register in the TWS parser tracking the precondition evaluation flag, asserted only when the input VDM state matches KERNEL privilege tags.
* **Mathematical Function**: Evaluates initial state predicates:
  $$P_{pre} = \text{CheckTag}(State_{init})$$
* **Visual / Geometric Modulation**: Governs the frequency multiplier ($f_x$) of the X-axis coordinate in the Lissajous projection, scaling the horizontal twists of the wireframe.

### HoarePost
* **VM Register Context**: Register in the TWS code generator tracking the postcondition invariant check flag, evaluated immediately after symbolic program execution.
* **Mathematical Function**: Verifies output state invariants:
  $$P_{post} = \begin{cases} 1 & \text{if } State_{final} \ge 0 \\ 0 & \text{otherwise} \end{cases}$$
* **Visual / Geometric Modulation**: Modulates the shear distortion matrix applied to the 3D wireframe envelope, shearing the projected coordinate grid into structural cutoff if the invariant is violated.
