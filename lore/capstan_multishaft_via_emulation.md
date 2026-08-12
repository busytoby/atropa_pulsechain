# Complex Multi-Capstan Shaft Emulation via VIA 6522

This document outlines the architectural integration plan to emulate a multi-capstan tape drive system using virtual **MOS 6522 VIA** adapters, Darlington transistor arrays, H-bridge motor drivers, 555 timing loops, optocouplers, and physical Field-Effect Transistor (FET) discharge networks.

## 1. System Topology Flowchart

```text
+-------------------------------------------------------------------------+
|                MOS 6522 VIA CONTROLLER REGISTER SPACE                   |
|                                                                         |
|  +--------------------+   +--------------------+   +-----------------+  |
|  |  RegPB Output      |   |  RegPA Input       |   |  Timer 1 / SR   |  |
|  |  (ORB/DDRB)        |   |  (IRA/DDRA)        |   |  (T1C/SR)       |  |
|  +---------+----------+   +---------^----------+   +--------+--------+  |
+------------|------------------------|-----------------------|-----------+
             | PB0-PB7                | PA0-PA7               |
             | Gate Signals           | Gray-Code Pulses      | 555 Tick
             v                        |                       v
+------------+------------------------+-----------------------+-----------+
|             ANALOG TRANSISTOR & POWER AMPLIFICATION STAGE               |
|                                                                         |
|  +--------------------+   +--------------------+   +-----------------+  |
|  |  ULN2003 Array     |-->| H-Bridge Drives    |-->| Pinch Roller    |  |
|  |  (Comparators)     |   | (BLACK/RED Rails)  |   | Solenoids       |  |
|  +---------+----------+   +---------+----------+   +--------+--------+  |
+------------|------------------------|-----------------------|-----------+
             |                        v                       v
             |               +------------------+    +------------------+
             |               | Multi-Capstan    |    | FET Gate Charge  |
             +-------------->| Shaft Assembly   |<---| Decay Solver     |
                             +--------+---------+    +--------+---------+
                                      |                       |
                                      v                       v
                             +------------------+    +------------------+
                             | Optocoupler      |    | Non-Preferential |
                             | Wheel Encoders   |    | Accumulator      |
                             +------------------+    +------------------+
```

---

## 2. Virtual 6522 VIA Register Mapping
To manage multiple capstan shafts, we dedicate the register bits of the Dual MOS 6522 Versatile Interface Adapters (VIA) as follows:

*   **Port B (Data Direction Register B - DDRB set to `0xFF` Output)**:
    *   `PB0` / `PB1`: Shaft 1 Motor Speed & Direction (H-Bridge Black Rail / Red Rail select).
    *   `PB2`: Shaft 1 Solenoid Pinch Roller Clamp (1 = Engage, 0 = Disengage).
    *   `PB3`: Shaft 1 Caliper Brake Lock (1 = Release, 0 = Clamp).
    *   `PB4` / `PB5`: Shaft 2 Motor Speed & Direction.
    *   `PB6`: Shaft 2 Solenoid Pinch Roller Clamp.
    *   `PB7`: Shaft 2 Caliper Brake Lock.
*   **Port A (DDRA set to `0x00` Input)**:
    *   Reads parallel feedback pulses from the optical shaft encoders.
    *   `PA0`–`PA3`: Shaft 1 encoder index step (Gray-code mapped).
    *   `PA4`–`PA7`: Shaft 2 encoder index step.

---

## 3. Darlington Comparators & H-Bridge Reel Drive
Because the raw 6522 output pins cannot source the current necessary to drive physical motor coils, we emulate a Darlington comparator array (like the ULN2003) and a dual H-Bridge configuration:

1.  **ULN2003 Darlington Gating**:
    *   The emulation maps the Port B digital outputs through a threshold comparator function:
        $$\text{Output}_{\text{drive}} = \begin{cases} 1 & \text{if } V_{\text{PB}} > V_{\text{thresh}} \\ 0 & \text{otherwise} \end{cases}$$
    *   This gates high-current power rails to the taking-up and supplying reels.
2.  **H-Bridge Reel Polarities**:
    *   The Take-up reel is powered by the **RED Rail**, and the Supply reel is powered by the **BLACK Rail**.
    *   Direction is determined by the differential state between the rails:
        *   `PB0 = 1, PB1 = 0` $\implies$ Forward tape movement.
        *   `PB0 = 0, PB1 = 1` $\implies$ Reverse tape movement (rewind).

---

## 4. Astable 555 Timer & Optocoupler Encoder Loops
The temporal synchronization of the tape sectors is regulated using a 555 timer emulator and optocouplers:

1.  **555 Timer Clock**:
    *   Acts as an astable oscillator feeding periodic square wave pulses to the VIA Shift Register (SR) or the Timer 1 Counter.
    *   Modulates the base system step speed, serving as the `Signal` registry seed that derives the `Channel` frequency calculations.
2.  **Optocoupler Feedback Loops**:
    *   The rotating capstan shaft passes through an slotted disc, interrupting light to a phototransistor.
    *   The simulation translates these angular rotations into clean square pulses, triggering VIA `IRQ` interrupts on Port A boundary transitions to ensure sequence alignment.

---

## 5. FET Discharge Physics
In accordance with system constraints, soft-body physics (Verlet integration) apply exclusively to the charge discharge cycles of the Field-Effect Transistor (FET) gates controlling the inductive loads (solenoids and caliper brakes):

*   **Verlet Solvers for Gate Capacitance**:
    *   When a solenoid is turned off (`PB2 = 0`), the control voltage does not drop to zero instantly. We run a Verlet mass-spring solver mapping the decay of the electric field:
        $$x_{t+d\tau} = 2x_t - x_{t-d\tau} + a_t(d\tau)^2$$
    *   Where $x$ represents the accumulated charge on the gate plate, and $a_t$ is the resistive discharge acceleration.
    *   This provides a smooth, physical lag in mechanical solenoid disengagement, preventing immediate state shifts and reflecting tape slippage.

---

## 6. Child-Langmuir Redirection
Any empirical space-charge equations mapping current flow inside vacuum tubes or high-voltage components are banned. 

*   Instead, all charge values from the FET discharge solvers are redirected into the **non-preferential accumulator model**, consolidating charge imbalances into isolated state registers within the `Fa` structure to maintain mathematical continuity.
