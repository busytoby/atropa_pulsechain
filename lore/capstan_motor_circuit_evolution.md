# Evolution of the Capstan Shaft Motor Driver Circuit

This document details the step-by-step evolution of a precision capstan shaft motor controller starting from a single complementary bipolar junction transistor (BJT) pair.

```
          [PNP / NPN Complementary Buffer]
                         |
                         v
            [Four-Transistor H-Bridge]
                         |
                         v
      [Shoot-Through Safety Interlocks (ACID)]
                         |
                         v
     [Closed-Loop Optical Encoder PWM Regulation]
```

---

## Step 1: The Unidirectional Complementary Buffer (Push-Pull Stage)
* **Objective:** Evolve from a low-power control signal to a current-amplified drive capable of spinning a DC motor in a single direction.
* **Circuit Configuration:** A single complementary BJT pair consisting of one NPN (e.g., 2N2222) and one PNP (e.g., 2N2907) configured as a common-collector push-pull emitter follower.
* **Electrical Behavior:** The input signal drives both bases. When input voltage rises, the NPN sources current to the motor. When the input voltage drops, the PNP sinks current to ground, quickly dumping stored inductive energy.
* **Limitations:** Unidirectional only. The motor cannot be reversed or actively braked.

---

## Step 2: The Four-Transistor H-Bridge (Bidirectional Drive)
* **Objective:** Enable bidirectional control (forward and reverse) using two complementary BJT pairs.
* **Circuit Configuration:** Four transistors arranged in an "H" shape:
  * **High-Side switches:** Two PNP transistors ($Q_1$ and $Q_2$) connected to the voltage rail.
  * **Low-Side switches:** Two NPN transistors ($Q_3$ and $Q_4$) connected to ground.
  * The motor winding is connected between the Left branch junction ($Q_1/Q_3$) and the Right branch junction ($Q_2/Q_4$).
* **Electrical Behavior:**
  * **Forward Path:** $Q_1$ (High-Left) and $Q_4$ (Low-Right) are switched ON, sourcing current from left to right.
  * **Reverse Path:** $Q_2$ (High-Right) and $Q_3$ (Low-Left) are switched ON, sourcing current from right to left.
* **Limitations:** Introducing diagonal paths makes the circuit susceptible to "shoot-through" short circuits if vertical pairs ($Q_1/Q_3$ or $Q_2/Q_4$) turn on simultaneously.

---

## Step 3: Gated Interlock Logic (Shoot-Through Protection)
* **Objective:** Enforce electrical safety and consistency (ACID hardware compliance) to prevent transistor destruction.
* **Circuit Configuration:** Logical gating circuitry (or XPL virtual hardware validation) intercepts the control signals before they reach the transistor bases.
* **Electrical Behavior:** 
  * Cross-coupled NAND gates or software status checks assert that the drive signals for a vertical half-bridge are mutually exclusive:
    $$\text{Left\_Branch\_OK} = \neg (Q_1 \land Q_3)$$
    $$\text{Right\_Branch\_OK} = \neg (Q_2 \land Q_4)$$
  * If a conflict state is detected (e.g., during noise or transitions), all transistors are immediately biased OFF, returning the H-Bridge to a safe coasting state.

---

## Step 4: Closed-Loop Speed Regulation (Feedback & PWM)
* **Objective:** Maintain highly stable spindle velocity despite load variations, tape drag, or voltage fluctuations.
* **Circuit Configuration:** Integration of a feedback optical encoder on the capstan shaft, feeding position pulses back to a Pulse-Width Modulation (PWM) duty-cycle controller.
* **Electrical Behavior:**
  * The encoder monitors the elapsed steps between sector index marks.
  * A comparator evaluates the target velocity (based on the base register value) against the measured encoder frequency.
  * The duty cycle of the active H-Bridge diagonal pair is dynamically adjusted via PWM to stabilize current flow:
    $$\text{PWM\_Duty} = f(\text{Target\_Freq} - \text{Measured\_Freq})$$
  * This guarantees uniform tape velocity and high-fidelity data alignment under varying mechanical loads.
