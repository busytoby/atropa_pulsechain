# QST Issue #11 Technical Analysis: Spark-Gap Closed Oscillation Loop

A physical modeling study of the closed transmitter oscillation loop described in the December 1916/January 1917 *QST* issues by "Dr. Radio" ("Efficient Short Wave Transmitting").

---

## 1. Historical & Technical Context

In the December 1916 issue of *QST* (Volume 1, Number 12), the pseudonymous contributor "Dr. Radio" published the first part of **"Efficient Short Wave Transmitting"**. The paper highlighted the extreme energy losses in the closed primary oscillation loop of spark-gap transmitters operating at the legal amateur wavelength limit of $200\text{ meters}$ ($\approx 1.5\text{ MHz}$).

The primary source of inefficiency was identified as the **parasitic lead inductance** ($L_{\text{lead}}$) of the wiring connecting the high-voltage condenser bank to the rotary spark gap and the primary oscillation transformer (the "helix"). This parasitic inductance shifted the transmitter frequency upward (to longer wavelengths) and absorbed energy that should have been radiated.

```mermaid
graph LR
    Condenser["Condenser Bank (C_bank)"] -->|"Lead Inductance L_lead"| Gap["Spark Gap (R_gap(t))"]
    Gap --> Helix["Primary Helix (L_helix)"]
```

---

## 2. Mathematical Loop Model

Under the zero-loss algebraic invariant ($\Phi = 0$), the primary closed loop contains the condenser capacitance ($C_{\text{bank}}$), the main transformer inductance ($L_{\text{helix}}$), the parasitic lead inductance ($L_{\text{lead}}$), and the non-linear spark-gap resistance ($R_{\text{gap}}$).

The differential equation governing loop current $i(t)$ and condenser voltage $v_c(t)$ is:

$$(L_{\text{helix}} + L_{\text{lead}}) \frac{di(t)}{dt} + R_{\text{gap}}(i, t) \cdot i(t) + v_c(t) = 0$$

$$\frac{dv_c(t)}{dt} = \frac{i(t)}{C_{\text{bank}}}$$

### Spark-Gap Resistance Dynamics
The spark-gap resistance $R_{\text{gap}}(t)$ is modeled as a dynamic threshold switch that breaks down when the condenser voltage exceeds the air gap breakdown threshold $V_{\text{breakdown}}$:

$$R_{\text{gap}}(t) = \begin{cases}
R_{\text{insulated}} & \text{if } |v_c(t)| < V_{\text{breakdown}} \text{ and not ionized} \\
R_{\text{arc}} + (R_{\text{insulated}} - R_{\text{arc}}) \cdot e^{-t_{\text{arc}} / \tau_{\text{ionization}}} & \text{once breakdown occurs}
\end{cases}$$

---

## 3. Physical Parameters for "Dr. Radio" Transmitter Simulation

| Parameter | Symbol | Value | Description |
| :--- | :---: | :---: | :--- |
| **Condenser Capacity** | $C_{\text{bank}}$ | $0.005\,\mu\text{F}$ | Glass plate/oil-immersed condenser |
| **Helix Inductance** | $L_{\text{helix}}$ | $10.0\,\mu\text{H}$ | Primary winding of oscillation transformer |
| **Parasitic Lead Inductance** | $L_{\text{lead}}$ | $0.5 - 2.5\,\mu\text{H}$ | High-voltage connecting wire |
| **Breakdown Voltage** | $V_{\text{breakdown}}$ | $15000.0\text{ V}$ | Spark gap sparkover potential |
| **Arc Resistance** | $R_{\text{arc}}$ | $0.2\,\Omega$ | Heavy ionized plasma resistance |
| **Insulated Resistance**| $R_{\text{insulated}}$ | $10.0\text{ M}\Omega$ | Open air gap resistance |
| **Ionization Time** | $\tau_{\text{ionization}}$ | $5.0\text{ ns}$ | Spark ionization decay constant |
