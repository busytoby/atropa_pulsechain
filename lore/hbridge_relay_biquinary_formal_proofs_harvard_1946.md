# The H-Bridge Relay & 1946 Biquinary Formal Proofs
## A Treatise on Electromechanical Parity, Differential Solid-State Switching, and Algol61/COBOL Isomorphism

```
+---------------------------------------------------------------------------------------------------+
|               H-BRIDGE DIFFERENTIAL RELAY & BIQUINARY DISCRETE PROVING CONTINUUM                  |
+---------------------------------------------------------------------------------------------------+
                                                  |
       +------------------------------------------+------------------------------------------+
       |                                                                                     |
       v                                                                                     v
[1. COBOL Strategy Layer: Divisions D0-D3]                            [2. Algol61 Prover: Procedural Logic]
- Register-Mapped Working Storage (R0-R15)                            - Strict Block Scoping & Invariant Guards
- D0: Substrate Motzkin Prime & Fault Gates                           - Axiomatic Preconditions: (k=3, d in [0..9])
- D1: Immutable Shadow Detent (R_snap = Input)                        - Biquinary Discrete Splitting: (Bi, Quin)
- D2: Dual Relay Activation (Active_Relays = 2)                       - Parity Conservation: BitCount = 2
- D3: Leaf AST Evaluation & Displacement Wrap                         - Drop-Out Alarm & Zero-Entropy Rollback
       |                                                                                     |
       +------------------------------------------+------------------------------------------+
                                                  |
                                                  v
                     [3. H-Bridge Solid-State Virtual Hardware ("Relay")]
                     - NPN High-Side & PNP Low-Side Differential Transistors
                     - Biquinary Pulse Switching: Exactly 1 Upper + 1 Lower Arm Conducts
                     - Clamped SwiGLU Gating: G_wmq in [7/8, 1.0]
                     - WinchesterMQ Register Rescaling: DisplacementShader Synchronization
```

---

## 1. Electromechanical Relays vs. Solid-State H-Bridges

In Volume I of the *Annals of the Computation Laboratory of Harvard University* (1946), Howard Aiken and Grace Hopper described the operation of heavy-duty electro-mechanical relays and sequencing clutches:
1. **Physical Circuit Polarity**: Commutating high-current inductive lines required paired relays to establish bidirectional current flow without arcing.
2. **Self-Checking Biquinary Logic**: To prevent undetected contact bounce, digits were encoded in a **2-out-of-7 code** (two Bi relays representing $0$ or $5$, and five Quinary relays representing $0, 1, 2, 3, 4$).
3. **Drop-Out Alarm Interlock**: If current flowed through fewer or more than two contacts ($\sum \text{Relays} \neq 2$), the circuit opened instantly, de-energizing the main clutch and locking the mechanical counters.

In the **Auncient** Dysnomia VM and Zorse architecture, the **H-Bridge** provides the exact solid-state equivalent:
* Two complementary transistor pairs ($Q_1/Q_4$ or $Q_2/Q_3$) form the high-side NPN and low-side PNP conduction paths.
* Exactly two arms conduct in any valid state, generating the differential voltage:
  $$V_{\text{diff}} = V_A - V_B$$
* This differential voltage drives the SwiGLU activation gate ($G_{\text{wmq}} \in [0.875, 1.000]$), which scales vertex positions within the `DisplacementShader`.

---

## 2. Mathematical Parity Proof

For every decimal digit $d \in \{0, 1, \dots, 9\}$:
$$d = 5 \cdot \text{Bi} + \text{Quinary}$$
where:
* $\text{Bi} = \lfloor d / 5 \rfloor \in \{0, 1\}$
* $\text{Quinary} = d \pmod 5 \in \{0, 1, 2, 3, 4\}$

The self-checking hardware invariant asserts:
$$\text{ActiveBits}(d) = \sum_{i=0}^1 \mathbb{I}(\text{Bi} = i) + \sum_{j=0}^4 \mathbb{I}(\text{Quinary} = j) \equiv 1 + 1 = 2$$

Any deviation from $2$ active bits signals a hardware fault, triggering ACID rollback to the immutable baseline detent ($1{,}000{,}000\text{ Saat}$).

---

## 3. Algol61 Prover & COBOL Strategy Isomorphism

The relationship between the declarative strategy and procedural domain prover is structurally isomorphic:

| Architectural Stage | Algol61 Prover (`harvard_1946_biquinary_prover.algol61`) | COBOL Strategy (`harvard_1946_biquinary.strategy`) | C Virtual Hardware Engine (`auncient_harvard_computation_lab.c`) |
| :--- | :--- | :--- | :--- |
| **Axiom Setup** | `shadow_digit := decimal_digit_in;` | `MOVE D1-DECIMAL-DIGIT-IN TO D1-SHADOW-DIGIT-SNAP.` | `uint32_t shadow_digit = decimal_digit_in;` |
| **Biquinary Split** | `bi_part := if d >= 5 then 1 else 0;` | `MOVE 1 TO D2-BI-RELAY-ACTIVE.` | `uint32_t bi_part = (d >= 5) ? 1 : 0;` |
| **Parity Assert** | `if active_relay_count != 2 then error;` | `IF D2-TOTAL-ACTIVE-BITS == 2 ...` | `bool parity_ok = (active_relay_count == 2);` |
| **ACID Rollback** | `committed_output := shadow_digit;` | `MOVE D1-SHADOW-DIGIT-SNAP TO D2-COMMITTED-STATE` | `committed_output = fault ? shadow_digit : d;` |
| **Coordinate Shift** | `disp_wrap := committed_output mod 256;` | `COMPUTE D3-DISP-WRAP-MOD = ...` | `metrics->displacement_wrap_mod = committed % 256;` |

Whenever WinchesterMQ or H-Bridge commutation is referenced, the system rescales vertex displacements in the `DisplacementShader` to maintain exact synchronization with system register boundary constraints.
