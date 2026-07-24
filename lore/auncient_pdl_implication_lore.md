# The Gating of Implication and the PDL Converse Sandbox

## 1. The Clearance Bus of z/VM
In the early mainframe architectures of the Dysnomia VM, transaction scheduling required a way to enforce system-wide safety invariants without software interrupt overhead. Propositional Logic (PL) formulas defined security policies (e.g., that only kernel-level operations could access the WinchesterMQ registry), but translating these rules into software checks introduced timing jitter, exposing execution timing side-channels.

The solution came through the direct compilation of PL implications ($\phi \implies \psi$) into Propositional Dynamic Logic (PDL) joint test programs ($?\phi ; ?\psi$) wired directly to the NPN Black and PNP Red transistor gates of the PL/EXUS ALU.

```
                   z/VM Supervisor Access Request
                                 │
                                 ▼
                     [ PDL Precondition ?phi ] ──► (Verify User Tag == KERNEL)
                                 │
                 ┌───────────────┴───────────────┐
                 ▼ (Pass)                        ▼ (Fail)
      [ PDL Postcondition ?psi ]          Cutoff Gate (0)
     (Verify Target Bounds)                      │
                 │                               ▼
         ┌───────┴───────┐                 Halt Execution
         ▼ (Pass)        ▼ (Fail)
   Conduc Gate (1)  Cutoff Gate (0)
         │
         ▼
    Commit Write
```

By routing the output of precondition $\phi$ directly as the base voltage input of the postcondition $\psi$ test transistor, the ALU achieved branchless, constant-time clearance gating.

## 2. Converse Recovery and the Kleene Room
If a transaction program $\alpha$ violates a postcondition contract (modeled in PDL as $\alpha ; ?\psi$), the supervisor executes the converse program $\alpha^-$. Using shadow registries inside the 2-3 tree DAT, the converse program pops the before-images of modified registers and restores the VDM memory slice to its exact prior state.

For iterative loops ($\alpha^*$), z/VM establishes the **Kleene Room**—an isolated VDM partition where execution steps are tracked by the Wortman bounds checker. If the loop step count exceeds the safety threshold, the write gate drops to `TRANSISTOR_CUTOFF`, preventing infinite-loop CPU exhaustion.

---

## 3. Glossary Modulation Specifications

### GuardEvent
* **VM Register Context**: Register in the partition state scheduler containing the PL/I event flag associated with a PDL test program, asserted high only when preconditions are met.
* **Mathematical Function**: The state truth valuation mapping:
  $$E_{guard} = \begin{cases} 1 & \text{if } \phi \text{ holds} \\ 0 & \text{otherwise} \end{cases}$$
* **Visual / Geometric Modulation**: Scales the lines thickness and line opacity of the projected 3D wireframe envelope, fading the lines to zero if the guard evaluates to false.

### ConverseStack
* **VM Register Context**: Stack pointer register tracking the address layout of shadow before-images allocated for converse transaction recovery ($\alpha^-$).
* **Mathematical Function**: Stacks register update records recursively:
  $$S_{recovery} = \{ (Key_i, Val_i, Hash_i) \}$$
* **Visual / Geometric Modulation**: Alters the rotation axis phase shift ($\phi_w$) of the coordinate projection, winding the 3D wireframe coordinates in reverse during rollback phases.
